#include "FuelCalculator.h"
#include <numeric>

double FuelCalculator::calculateInstantRate(double prevFuel, double currFuel, double elapsedSeconds) {
    if (elapsedSeconds <= 0.0) {
        return -1.0;
    }
    return (prevFuel - currFuel) / elapsedSeconds;
}

double FuelCalculator::updateRunningAverage(double currentAvg, double newRate, int sampleCount) {
    if (sampleCount <= 0) {
        return newRate;
    }
    return currentAvg + (newRate - currentAvg) / static_cast<double>(sampleCount);
}

double FuelCalculator::computeFlightAverage(const AirplaneRecord& record) {
    return record.currentFlightAvg;
}

double FuelCalculator::computeHistoricalAverage(const AirplaneRecord& record) {
    if (record.completedFlightAverages.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (double avg : record.completedFlightAverages) {
        sum += avg;
    }
    return sum / static_cast<double>(record.completedFlightAverages.size());
}
