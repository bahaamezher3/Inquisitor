#include "Inquisitor.hpp"
#include "arp/ArpPoisoner.hpp"
#include "network/PacketCapture.hpp"
#include "protocol/FtpParser.hpp"
#include <iostream>
#include <thread>
#include <chrono>

Inquisitor::Inquisitor(const Config &config)
	: config_(config), isRunning_(false) {}

Inquisitor::~Inquisitor()
{
	stopAttack();
}
bool Inquisitor::initialize()
{
	try
	{
		ArpPoisoner_ = std::make_unique<ArpPoisoner>(config_.interface);
		ftpParser_ = std::make_unique<FtpParser>(config_.verbose);
		PacketCapture_ = std::make_unique<PacketCapture>(config_.interface, ftpParser_.get());
		return true;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Initialization failed: " << e.what() << std::endl;
		return false;
	}
}
void Inquisitor::startAttack()
{
	isRunning_ = true;

}