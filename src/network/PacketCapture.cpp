#include "PacketCapture.hpp"
#include "../protocol/FtpParser.hpp"
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>
#include <iostream>
#include <cstring>

PacketCapture::PacketCapture(const std::string& interface, FtpParser* parser)
    : interface_(interface), ftpParser_(parser), handle_(nullptr), isCapturing_(false) {
}

PacketCapture::~PacketCapture() {
    stopCapture();
}

void PacketCapture::startCapture() {
    char errbuf[PCAP_ERRBUF_SIZE];
    
    handle_ = pcap_open_live(interface_.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (handle_ == nullptr) {
        std::cerr << "Failed to open device: " << errbuf << std::endl;
        return;
    }
    
    struct bpf_program fp;
    const char* filter = "tcp port 21 or tcp port 20";
    
    if (pcap_compile(handle_, &fp, filter, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        std::cerr << "Failed to compile filter" << std::endl;
        return;
    }
    
    if (pcap_setfilter(handle_, &fp) == -1) {
        std::cerr << "Failed to set filter" << std::endl;
        return;
    }
    
    pcap_freecode(&fp);
    
    isCapturing_ = true;
    std::cout << "Packet capture started on " << interface_ << std::endl;
    
    pcap_loop(handle_, -1, packetHandler, (u_char*)ftpParser_);
}

void PacketCapture::stopCapture() {
    if (handle_) {
        pcap_breakloop(handle_);
        pcap_close(handle_);
        handle_ = nullptr;
    }
    isCapturing_ = false;
}

void PacketCapture::packetHandler(u_char* userData, const struct pcap_pkthdr* pkthdr,
                                  const u_char* packet) {
    FtpParser* parser = (FtpParser*)userData;
    
    struct ether_header* ethHeader = (struct ether_header*)packet;
    if (ntohs(ethHeader->ether_type) != ETHERTYPE_IP) {
        return;
    }
    
    struct ip* ipHeader = (struct ip*)(packet + sizeof(struct ether_header));
    if (ipHeader->ip_p != IPPROTO_TCP) {
        return;
    }
    
    int ipHeaderLen = ipHeader->ip_hl * 4;
    struct tcphdr* tcpHeader = (struct tcphdr*)((u_char*)ipHeader + ipHeaderLen);
    int tcpHeaderLen = tcpHeader->th_off * 4;
    
    const u_char* payload = (u_char*)tcpHeader + tcpHeaderLen;
    int payloadLen = ntohs(ipHeader->ip_len) - ipHeaderLen - tcpHeaderLen;
    
    if (payloadLen > 0) {
        parser->parsePayload(payload, payloadLen);
    }
}