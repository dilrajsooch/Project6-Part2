#pragma once
#include <string>

struct TelemetryDataPoint {
    std::string timestamp; // ISO 8601 string "2023-03-03T14:53:21"
    double fuelRemaining;
};
