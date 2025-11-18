#include "InquisitorFacade.hpp"
#include <iostream>
#include <cstring>
#include <csignal>
#include <regex>
#include <unistd.h>

InquisitorFacade* g_facade = nullptr;

void signalHandler(int signum) {
    std::cout << "\nCaught signal " << signum << ", cleaning up..." << std::endl;
    if (g_facade) {
        g_facade->stopAttack();
    }
    exit(0);
}

bool isValidIPv4(const std::string& ip) {
    std::regex ipPattern(
        R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
    );
    return std::regex_match(ip, ipPattern);
}

bool isValidMAC(const std::string& mac) {
    std::regex macPattern(R"(^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$)");
    return std::regex_match(mac, macPattern);
}

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName 
              << " <IP-src> <MAC-src> <IP-target> <MAC-target> [-v]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  -v    Verbose mode (show all FTP traffic)" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << progName 
              << " 192.168.1.1 00:11:22:33:44:55 192.168.1.100 aa:bb:cc:dd:ee:ff" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Error: Insufficient arguments" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    if (geteuid() != 0) {
        std::cerr << "Error: This program requires root privileges" << std::endl;
        return 1;
    }

    std::string ipSrc = argv[1];
    std::string macSrc = argv[2];
    std::string ipTarget = argv[3];
    std::string macTarget = argv[4];
    bool verbose = false;

    if (argc > 5 && std::strcmp(argv[5], "-v") == 0) {
        verbose = true;
    }

    if (!isValidIPv4(ipSrc)) {
        std::cerr << "Error: Invalid source IP: " << ipSrc << std::endl;
        return 1;
    }
    if (!isValidIPv4(ipTarget)) {
        std::cerr << "Error: Invalid target IP: " << ipTarget << std::endl;
        return 1;
    }
    if (!isValidMAC(macSrc)) {
        std::cerr << "Error: Invalid source MAC: " << macSrc << std::endl;
        return 1;
    }
    if (!isValidMAC(macTarget)) {
        std::cerr << "Error: Invalid target MAC: " << macTarget << std::endl;
        return 1;
    }

    try {
        InquisitorFacade::Config config{
            ipSrc, macSrc, ipTarget, macTarget, verbose, "eth0"
        };

        InquisitorFacade facade(config);
        g_facade = &facade;

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        std::cout << "Inquisitor - ARP Poisoning Tool" << std::endl;
        std::cout << "================================" << std::endl;
        std::cout << "Source:  " << ipSrc << " (" << macSrc << ")" << std::endl;
        std::cout << "Target:  " << ipTarget << " (" << macTarget << ")" << std::endl;
        std::cout << "Verbose: " << (verbose ? "enabled" : "disabled") << std::endl;
        std::cout << "================================" << std::endl;

        if (!facade.initialize()) {
            std::cerr << "Error: Failed to initialize" << std::endl;
            return 1;
        }

        std::cout << "Starting ARP poisoning..." << std::endl;
        std::cout << "Press CTRL+C to stop" << std::endl;

        facade.startAttack();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}