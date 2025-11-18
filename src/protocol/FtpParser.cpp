#include "FtpParser.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>

FtpParser::FtpParser(bool verbose) : verbose_(verbose) {
}

void FtpParser::parsePayload(const uint8_t* payload, int length) {
    std::string data((char*)payload, length);
    buffer_ += data;
    
    size_t pos;
    while ((pos = buffer_.find("\r\n")) != std::string::npos) {
        std::string command = buffer_.substr(0, pos);
        buffer_.erase(0, pos + 2);
        
        if (!command.empty()) {
            processCommand(command);
        }
    }
}

void FtpParser::processCommand(const std::string& command) {
    if (verbose_) {
        std::cout << "[FTP] " << command << std::endl;
        return;
    }
    
    if (command.find("RETR ") == 0) {
        std::string filename = extractFilename(command);
        std::cout << "[File Download] " << filename << std::endl;
    } else if (command.find("STOR ") == 0) {
        std::string filename = extractFilename(command);
        std::cout << "[File Upload] " << filename << std::endl;
    } else if (command.find("USER ") == 0) {
        std::string user = command.substr(5);
        std::cout << "[Login] Username: " << user << std::endl;
    } else if (command.find("PASS ") == 0) {
        std::string pass = command.substr(5);
        std::cout << "[Login] Password: " << pass << std::endl;
    }
}

bool FtpParser::isFileTransferCommand(const std::string& cmd) {
    return cmd.find("RETR") == 0 || cmd.find("STOR") == 0;
}

std::string FtpParser::extractFilename(const std::string& command) {
    size_t spacePos = command.find(' ');
    if (spacePos != std::string::npos) {
        return command.substr(spacePos + 1);
    }
    return "";
}
