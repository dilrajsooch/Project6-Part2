#include <winsock2.h>
#include <ws2tcpip.h>
#include "TcpTransmitter.h"
#include <iostream>

TcpTransmitter::TcpTransmitter()
    : sock(INVALID_SOCKET), connected(false) {
}

TcpTransmitter::~TcpTransmitter() {
    disconnect();
}

bool TcpTransmitter::connect(const std::string& serverIp, int port) {
    // Initialize Winsock
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        std::cerr << "WSAStartup failed: " << wsaResult << std::endl;
        return false;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    // Resolve server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons(static_cast<u_short>(port));

    int convResult = inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
    if (convResult != 1) {
        // Try resolving as hostname
        struct addrinfo hints{};
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        struct addrinfo* result = nullptr;
        if (getaddrinfo(serverIp.c_str(), std::to_string(port).c_str(), &hints, &result) != 0 || result == nullptr) {
            std::cerr << "Failed to resolve server address: " << serverIp << std::endl;
            closesocket(sock);
            sock = INVALID_SOCKET;
            WSACleanup();
            return false;
        }
        serverAddr = *reinterpret_cast<sockaddr_in*>(result->ai_addr);
        freeaddrinfo(result);
    }

    // Connect
    int connectResult = ::connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (connectResult == SOCKET_ERROR) {
        std::cerr << "connect() failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    connected = true;
    return true;
}

bool TcpTransmitter::send(const std::string& packet) {
    if (!connected || sock == INVALID_SOCKET) return false;

    int totalSent = 0;
    int dataLen   = static_cast<int>(packet.size());
    const char* data = packet.c_str();

    while (totalSent < dataLen) {
        int sent = ::send(sock, data + totalSent, dataLen - totalSent, 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
            connected = false;
            return false;
        }
        totalSent += sent;
    }

    return true;
}

void TcpTransmitter::disconnect() {
    if (sock != INVALID_SOCKET) {
        shutdown(sock, SD_BOTH);
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
    }
    connected = false;
}

bool TcpTransmitter::isConnected() const {
    return connected;
}
