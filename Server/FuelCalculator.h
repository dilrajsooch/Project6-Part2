#pragma once
#include "AirplaneRecord.h"

class FuelCalculator {
public:
    // Returns gallons/second. Returns -1 if elapsed <= 0.
    static double calculateInstantRate(double prevFuel, double currFuel, double elapsedSeconds);
    // Cumulative moving average
    static double updateRunningAverage(double currentAvg, double newRate, int sampleCount);
    static double computeFlightAverage(const AirplaneRecord& record);
    static double computeHistoricalAverage(const AirplaneRecord& record);
};
