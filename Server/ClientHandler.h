#pragma once
#include <winsock2.h>
#include "FleetDataManager.h"

class ClientHandler {
public:
    // Called in a new thread. Owns the socket.
    static void handleClient(SOCKET clientSocket, FleetDataManager* manager);

private:
    // Parse "AIRPLANE_ID|TIMESTAMP|FUEL_REMAINING"
    // Returns false if malformed
    static bool parsePacket(const std::string& line, std::string& airplaneId,
                            std::string& timestamp, double& fuelRemaining);
};
