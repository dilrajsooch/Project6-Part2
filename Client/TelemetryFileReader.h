#pragma once
#include <string>
#include <fstream>
#include <optional>
#include "TelemetryDataPoint.h"

class TelemetryFileReader {
public:
    bool open(const std::string& filePath);
    // Returns empty optional at EOF or error
    std::optional<TelemetryDataPoint> readNext();
    void close();
    ~TelemetryFileReader();

private:
    std::ifstream fileStream;
    bool headerParsed = false;
    TelemetryDataPoint bufferedHeader; // first data point from header line
    bool hasBuffered = false;

    // Parse "D_M_YYYY HH:MM:SS" -> "YYYY-MM-DDTHH:MM:SS"
    std::string parseTimestamp(const std::string& raw);
    // Parse a data line " D_M_YYYY HH:MM:SS,fuel_value,"
    std::optional<TelemetryDataPoint> parseLine(const std::string& line);
};
