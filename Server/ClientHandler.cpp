#include <winsock2.h>
#include "ClientHandler.h"
#include "Logger.h"
#include <string>
#include <sstream>

bool ClientHandler::parsePacket(const std::string& line, std::string& airplaneId,
                                 std::string& timestamp, double& fuelRemaining) {
    // Expected format: "AIRPLANE_ID|TIMESTAMP|FUEL_REMAINING"
    if (line.empty()) return false;

    // Find first '|'
    std::size_t pos1 = line.find('|');
    if (pos1 == std::string::npos) return false;

    // Find second '|'
    std::size_t pos2 = line.find('|', pos1 + 1);
    if (pos2 == std::string::npos) return false;

    airplaneId = line.substr(0, pos1);
    timestamp  = line.substr(pos1 + 1, pos2 - pos1 - 1);

    std::string fuelStr = line.substr(pos2 + 1);
    // Trim any trailing whitespace/CR
    while (!fuelStr.empty() && (fuelStr.back() == '\r' || fuelStr.back() == '\n' || fuelStr.back() == ' ')) {
        fuelStr.pop_back();
    }

    if (airplaneId.empty() || timestamp.empty() || fuelStr.empty()) return false;

    try {
        fuelRemaining = std::stod(fuelStr);
    } catch (...) {
        return false;
    }

    return true;
}

void ClientHandler::handleClient(SOCKET clientSocket, FleetDataManager* manager) {
    Logger::log("Client handler started for new connection.");

    std::string lastAirplaneId;
    std::string lineBuffer;

    while (true) {
        char ch = 0;
        int result = recv(clientSocket, &ch, 1, 0);

        if (result == 0) {
            // Graceful disconnect
            Logger::log("Client disconnected gracefully.");
            break;
        } else if (result == SOCKET_ERROR) {
            Logger::log("Socket error while receiving data.");
            break;
        }

        if (ch == '\n') {
            // Process the line
            // Strip trailing '\r' if present (CRLF line endings)
            if (!lineBuffer.empty() && lineBuffer.back() == '\r') {
                lineBuffer.pop_back();
            }

            if (!lineBuffer.empty()) {
                std::string airplaneId, timestamp;
                double fuelRemaining = 0.0;

                if (parsePacket(lineBuffer, airplaneId, timestamp, fuelRemaining)) {
                    lastAirplaneId = airplaneId;
                    manager->updateFuelReading(airplaneId, timestamp, fuelRemaining);
                } else {
                    Logger::log("Malformed packet received: " + lineBuffer);
                }
            }

            lineBuffer.clear();
        } else {
            lineBuffer += ch;
        }
    }

    // Finalize the flight for this client
    if (!lastAirplaneId.empty()) {
        Logger::log("Completing flight for aircraft: " + lastAirplaneId);
        manager->completeFlight(lastAirplaneId);
    }

    closesocket(clientSocket);
    Logger::log("Client socket closed.");
}
