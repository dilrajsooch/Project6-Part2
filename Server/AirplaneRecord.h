#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "TelemetryDataPoint.h"

struct AirplaneRecord {
    std::string airplaneId;
    double currentFlightAvg = 0.0;
    int currentFlightSampleCount = 0;
    bool hasPreviousPoint = false;
    TelemetryDataPoint previousDataPoint;
    std::vector<double> completedFlightAverages;
    bool isFlightActive = false;
    double totalFuelConsumed = 0.0;
    std::mutex recordMutex; // per-record lock

    // non-copyable due to mutex
    AirplaneRecord() = default;
    AirplaneRecord(const AirplaneRecord&) = delete;
    AirplaneRecord& operator=(const AirplaneRecord&) = delete;
};
