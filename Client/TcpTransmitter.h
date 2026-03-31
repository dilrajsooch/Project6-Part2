#pragma once
#include <winsock2.h>
#include <string>

class TcpTransmitter {
public:
    TcpTransmitter();
    ~TcpTransmitter();
    bool connect(const std::string& serverIp, int port);
    bool send(const std::string& packet);
    void disconnect();
    bool isConnected() const;

private:
    SOCKET sock = INVALID_SOCKET;
    bool connected = false;
};
