#pragma once
#include <string>
#include "TelemetryDataPoint.h"

class PacketBuilder {
public:
    std::string buildPacket(const std::string& airplaneId, const TelemetryDataPoint& data);
};
