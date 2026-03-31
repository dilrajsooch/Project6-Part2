#include "FleetDataManager.h"
#include "FuelCalculator.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <vector>

std::time_t FleetDataManager::parseIso8601(const std::string& timestamp) {
    // Format: "YYYY-MM-DDTHH:MM:SS"
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    if (std::sscanf(timestamp.c_str(), "%d-%d-%dT%d:%d:%d",
                    &year, &month, &day, &hour, &minute, &second) != 6) {
        return static_cast<std::time_t>(-1);
    }

    std::tm t{};
    t.tm_year  = year - 1900;
    t.tm_mon   = month - 1;
    t.tm_mday  = day;
    t.tm_hour  = hour;
    t.tm_min   = minute;
    t.tm_sec   = second;
    t.tm_isdst = -1;

    return std::mktime(&t);
}

AirplaneRecord& FleetDataManager::getOrCreateRecord(const std::string& id) {
    std::lock_guard<std::mutex> lock(mapMutex);
    auto it = records.find(id);
    if (it == records.end()) {
        auto record = std::make_unique<AirplaneRecord>();
        record->airplaneId = id;
        auto* ptr = record.get();
        records[id] = std::move(record);
        return *ptr;
    }
    return *(it->second);
}

void FleetDataManager::updateFuelReading(const std::string& id, const std::string& timestamp, double fuelRemaining) {
    AirplaneRecord& record = getOrCreateRecord(id);

    std::lock_guard<std::mutex> lock(record.recordMutex);

    if (record.hasPreviousPoint) {
        std::time_t prevTime = parseIso8601(record.previousDataPoint.timestamp);
        std::time_t currTime = parseIso8601(timestamp);

        if (prevTime != static_cast<std::time_t>(-1) && currTime != static_cast<std::time_t>(-1)) {
            double elapsedSeconds = std::difftime(currTime, prevTime);
            double rate = FuelCalculator::calculateInstantRate(
                record.previousDataPoint.fuelRemaining, fuelRemaining, elapsedSeconds);

            if (rate >= 0.0) {
                record.currentFlightSampleCount++;
                record.currentFlightAvg = FuelCalculator::updateRunningAverage(
                    record.currentFlightAvg, rate, record.currentFlightSampleCount);

                double consumed = record.previousDataPoint.fuelRemaining - fuelRemaining;
                if (consumed > 0.0) {
                    record.totalFuelConsumed += consumed;
                }
            }
        }
    }

    record.previousDataPoint.timestamp    = timestamp;
    record.previousDataPoint.fuelRemaining = fuelRemaining;
    record.hasPreviousPoint = true;
    record.isFlightActive   = true;
}

void FleetDataManager::completeFlight(const std::string& id) {
    // Use mapMutex to check existence
    AirplaneRecord* recordPtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex);
        auto it = records.find(id);
        if (it == records.end()) return;
        recordPtr = it->second.get();
    }

    std::lock_guard<std::mutex> lock(recordPtr->recordMutex);

    if (recordPtr->currentFlightSampleCount > 0) {
        recordPtr->completedFlightAverages.push_back(recordPtr->currentFlightAvg);

        std::ostringstream msg;
        msg << "Flight completed for " << id
            << " | Avg burn rate: " << std::fixed << std::setprecision(6)
            << recordPtr->currentFlightAvg << " gal/s"
            << " | Total consumed: " << std::fixed << std::setprecision(2)
            << recordPtr->totalFuelConsumed << " gal";
        Logger::log(msg.str());
    }

    recordPtr->currentFlightAvg          = 0.0;
    recordPtr->currentFlightSampleCount  = 0;
    recordPtr->hasPreviousPoint          = false;
    recordPtr->isFlightActive            = false;
    recordPtr->totalFuelConsumed         = 0.0;
}

void FleetDataManager::printFleetSummary() {
    // Take a snapshot of pointers to avoid holding mapMutex during printing
    std::vector<AirplaneRecord*> snapshot;
    {
        std::lock_guard<std::mutex> lock(mapMutex);
        for (auto& kv : records) {
            snapshot.push_back(kv.second.get());
        }
    }

    if (snapshot.empty()) {
        Logger::log("--- Fleet Summary: No aircraft tracked yet ---");
        return;
    }

    Logger::log("--- Fleet Summary ---");
    for (AirplaneRecord* rec : snapshot) {
        std::lock_guard<std::mutex> lock(rec->recordMutex);

        double historicalAvg = FuelCalculator::computeHistoricalAverage(*rec);

        std::ostringstream msg;
        msg << "Aircraft: " << rec->airplaneId
            << " | Current Avg Burn: " << std::fixed << std::setprecision(6)
            << rec->currentFlightAvg << " gal/s"
            << " | Completed Flights: " << rec->completedFlightAverages.size()
            << " | Historical Avg: " << std::fixed << std::setprecision(6)
            << historicalAvg << " gal/s"
            << " | Flight Active: " << (rec->isFlightActive ? "Yes" : "No");
        Logger::log(msg.str());
    }
    Logger::log("--- End Fleet Summary ---");
}
