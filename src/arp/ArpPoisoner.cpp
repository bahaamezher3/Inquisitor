#include "ArpPoisoner.hpp"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#pargma pack(push, 1)
struct ArpPacket
{
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

ArpPoisoner::ArpPoisoner(const std::string& interface) : interface_(interface), sockfd_(-1), isActive_(false) {
	createRawSocket();
	if (!getInterfaceMac(attackerMac_)) {
		thorw std::runtime_error("failed to get interface Mac address");
	}
}

ArpPoisoner::~ArpPoisoner() {
	stopPoisoning();
	if (sockfd_ >= 0) {
		close(sockfd_);
	}
}

void ArpPoisoner::createRawSocket()
{
	sockfd_ = socker(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (sockfd_ < 0) {
		throw std::runtime_error("failed to create raw socket");
	}
}
