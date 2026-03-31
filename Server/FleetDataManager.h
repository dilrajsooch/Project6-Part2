#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include "AirplaneRecord.h"

class FleetDataManager {
public:
    // Get or create record for airplane ID
    AirplaneRecord& getOrCreateRecord(const std::string& id);
    // Update fuel reading: compute delta, update running avg
    void updateFuelReading(const std::string& id, const std::string& timestamp, double fuelRemaining);
    // Finalize flight: store avg, reset accumulators
    void completeFlight(const std::string& id);
    // Print summary to console
    void printFleetSummary();

private:
    std::unordered_map<std::string, std::unique_ptr<AirplaneRecord>> records;
    std::mutex mapMutex; // protects map insertions

    // Parse ISO 8601 "YYYY-MM-DDTHH:MM:SS" to time_t
    static std::time_t parseIso8601(const std::string& timestamp);
};
