#include "PacketBuilder.h"
#include <sstream>
#include <iomanip>

std::string PacketBuilder::buildPacket(const std::string& airplaneId, const TelemetryDataPoint& data) {
    std::ostringstream oss;
    oss << airplaneId << "|"
        << data.timestamp << "|"
        << std::fixed << std::setprecision(6) << data.fuelRemaining
        << "\n";
    return oss.str();
}
