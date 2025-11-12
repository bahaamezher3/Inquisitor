#ifndef ARP_POISONER_HPP
#define ARP_POISONER_HPP

#include <string>
#include <cstdint>

class ArpPoisoner
{
public:
	ArpPoisoner(const std::string &interface);
	~ArpPoisoner();

	bool startPoisoning(const std::string &ipSrc, const std::string &macSrc,
						const std::string &ipTarget, const std::string &macTarget);
	void stopPoisoning();
	void maintainPoisoning();

private:
	std::string interface_;
	int sockfd_;
	bool isActive_;

	std::string ipSrc_, macSrc_;
	std::string ipTarget_, macTarget_;
	std::string attackerMac_;

	bool sendArpPacket(const std::string &srcIp, const std::string &srcMac,
					   const std::string &dstIp, const std::string &dstMac,
					   uint16_t operation);
	bool getInterfaceMac(std::string &outMac);
	void createRawSocket();
	void macStringToBytes(const std::string &Mac, uint8_t *bytes);
	uint32_t ipStringToInt(const std::string &ip);
};

#endif