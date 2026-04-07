#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include "ClientHandler.h"
#include "FleetDataManager.h"
#include "Logger.h"

// Periodic summary thread: prints fleet summary every 5 seconds
static void summaryThreadFunc(FleetDataManager* manager, std::atomic<bool>* running) {
    while (running->load()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (running->load()) {
            manager->printFleetSummary();
        }
    }
}

int main(int argc, char* argv[]) {
    int port = 5000;
    if (argc >= 2) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Invalid port argument. Using default port 5000." << std::endl;
        }
    }

    // Initialize Winsock
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        std::cerr << "WSAStartup failed with error: " << wsaResult << std::endl;
        return 1;
    }

    // Create listening socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Allow address reuse
    int optVal = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&optVal), sizeof(optVal));

    // Bind
    sockaddr_in serverAddr{};
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port        = htons(static_cast<u_short>(port));

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    Logger::log("Aircraft Fuel Monitor Server started on port " + std::to_string(port));

    FleetDataManager manager;
    std::atomic<int> activeConnections{0};
    std::atomic<bool> serverRunning{true};

    // Start summary background thread
    std::thread summaryThread(summaryThreadFunc, &manager, &serverRunning);

    // Accept loop
    while (true) {
        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);

        if (clientSocket == INVALID_SOCKET) {
            int err = WSAGetLastError();
            if (serverRunning.load()) {
                Logger::log("accept() failed: " + std::to_string(err));
            }
            break;
        }

        // Set a 30-second receive timeout so idle clients don't block threads forever
        DWORD recvTimeout = 30000;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&recvTimeout), sizeof(recvTimeout));

        int connCount = ++activeConnections;
        Logger::log("Client connected. Active: " + std::to_string(connCount));

        // Spawn a thread for this client
        std::thread clientThread([clientSocket, &manager, &activeConnections]() {
            ClientHandler::handleClient(clientSocket, &manager);
            int remaining = --activeConnections;
            Logger::log("Client disconnected. Active: " + std::to_string(remaining));
        });
        clientThread.detach();
    }

    serverRunning.store(false);
    closesocket(listenSocket);

    // Wait for all client threads to finish before destroying shared objects
    while (activeConnections.load() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    summaryThread.join();
    WSACleanup();

    Logger::log("Server shutting down.");
    return 0;
}
