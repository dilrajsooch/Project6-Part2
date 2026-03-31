#include <winsock2.h>
#include <iostream>
#include <string>
#include <random>
#include <iomanip>
#include <sstream>
#include "TelemetryFileReader.h"
#include "PacketBuilder.h"
#include "TcpTransmitter.h"

int main(int argc, char* argv[]) {
    // Parse arguments
    // Usage: Client.exe <serverIp> <port> <telemetryFile>
    std::string serverIp      = "127.0.0.1";
    int         port          = 5000;
    std::string telemetryFile = "katl-kefd-B737-700.txt";

    if (argc >= 2) serverIp      = argv[1];
    if (argc >= 3) {
        try {
            port = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "Invalid port argument. Using default port 5000." << std::endl;
        }
    }
    if (argc >= 4) telemetryFile = argv[3];

    // Generate random airplane ID: "AC-" + 4-digit number
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(1000, 9999);
    int randomNum = dist(rng);

    std::ostringstream idStream;
    idStream << "AC-" << std::setw(4) << std::setfill('0') << randomNum;
    std::string airplaneId = idStream.str();

    // Print startup info
    std::cout << "Airplane ID: " << airplaneId
              << " | Server: " << serverIp << ":" << port
              << " | File: " << telemetryFile << std::endl;

    // Open telemetry file
    TelemetryFileReader reader;
    if (!reader.open(telemetryFile)) {
        std::cerr << "Error: Could not open telemetry file: " << telemetryFile << std::endl;
        return 1;
    }

    // Connect to server
    TcpTransmitter transmitter;
    if (!transmitter.connect(serverIp, port)) {
        std::cerr << "Error: Could not connect to server at " << serverIp << ":" << port << std::endl;
        reader.close();
        return 1;
    }

    std::cout << "Connected to server. Transmitting telemetry..." << std::endl;

    PacketBuilder builder;
    int packetCount = 0;

    // Main loop: read -> build -> send
    while (true) {
        auto dataPoint = reader.readNext();
        if (!dataPoint.has_value()) {
            // EOF reached
            break;
        }

        std::string packet = builder.buildPacket(airplaneId, dataPoint.value());

        if (!transmitter.send(packet)) {
            std::cerr << "Error: Failed to send packet. Aborting transmission." << std::endl;
            break;
        }

        packetCount++;
    }

    // Clean up
    transmitter.disconnect();
    reader.close();

    std::cout << "Transmission complete. Airplane ID: " << airplaneId
              << " | Packets sent: " << packetCount << std::endl;

    return 0;
}
