#ifndef PACKET_CAPTURE_HPP
#define PACKET_CAPTURE_HPP

#include <string>
#include <pcap.h>

class FtpParser;

class PacketCapture {
public:
    PacketCapture(const std::string& interface, FtpParser* parser);
    ~PacketCapture();

    void startCapture();
    void stopCapture();

private:
    std::string interface_;
    FtpParser* ftpParser_;
    pcap_t* handle_;
    bool isCapturing_;
    
    static void packetHandler(u_char* userData, const struct pcap_pkthdr* pkthdr,
                             const u_char* packet);
};

#endif