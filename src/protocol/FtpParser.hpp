#ifndef FTP_PARSER_HPP
#define FTP_PARSER_HPP

#include <string>
#include <cstdint>

class FtpParser {
public:
    FtpParser(bool verbose);
    void parsePayload(const uint8_t* payload, int length);

private:
    bool verbose_;
    std::string buffer_;
    
    void processCommand(const std::string& command);
    bool isFileTransferCommand(const std::string& cmd);
    std::string extractFilename(const std::string& command);
};

#endif