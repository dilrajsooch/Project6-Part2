#include "TelemetryFileReader.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <vector>

// Trim leading and trailing whitespace from a string
static std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Split a string by a delimiter, returning all tokens
static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(s);
    while (std::getline(iss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string TelemetryFileReader::parseTimestamp(const std::string& raw) {
    // Input: "D_M_YYYY HH:MM:SS"  (e.g., "3_3_2023 14:53:21")
    // Output: "YYYY-MM-DDTHH:MM:SS" (e.g., "2023-03-03T14:53:21")

    std::string trimmed = trim(raw);

    // Find space separating date and time
    std::size_t spacePos = trimmed.find(' ');
    if (spacePos == std::string::npos) return "";

    std::string datePart = trimmed.substr(0, spacePos);   // "D_M_YYYY"
    std::string timePart = trimmed.substr(spacePos + 1);  // "HH:MM:SS"

    // Split date by '_'
    auto dateParts = split(datePart, '_');
    if (dateParts.size() < 3) return "";

    std::string day   = dateParts[0];
    std::string month = dateParts[1];
    std::string year  = dateParts[2];

    // Zero-pad day and month to 2 digits
    if (day.size() == 1)   day   = "0" + day;
    if (month.size() == 1) month = "0" + month;

    return year + "-" + month + "-" + day + "T" + timePart;
}

std::optional<TelemetryDataPoint> TelemetryFileReader::parseLine(const std::string& line) {
    // Format: " D_M_YYYY HH:MM:SS,fuel_value,"
    std::string trimmed = trim(line);
    if (trimmed.empty()) return std::nullopt;

    auto tokens = split(trimmed, ',');
    if (tokens.size() < 2) return std::nullopt;

    std::string timestampRaw = trim(tokens[0]);
    std::string fuelStr      = trim(tokens[1]);

    if (timestampRaw.empty() || fuelStr.empty()) return std::nullopt;

    std::string timestamp = parseTimestamp(timestampRaw);
    if (timestamp.empty()) return std::nullopt;

    double fuelRemaining = 0.0;
    try {
        fuelRemaining = std::stod(fuelStr);
    } catch (...) {
        return std::nullopt;
    }

    return TelemetryDataPoint{timestamp, fuelRemaining};
}

bool TelemetryFileReader::open(const std::string& filePath) {
    fileStream.open(filePath);
    if (!fileStream.is_open()) return false;

    // Read and parse header line
    // Format: "FUEL TOTAL QUANTITY,D_M_YYYY HH:MM:SS,fuel_value,"
    std::string headerLine;
    if (!std::getline(fileStream, headerLine)) {
        fileStream.close();
        return false;
    }

    // Strip trailing CR if present
    if (!headerLine.empty() && headerLine.back() == '\r') {
        headerLine.pop_back();
    }

    auto tokens = split(headerLine, ',');
    // tokens[0] = "FUEL TOTAL QUANTITY", tokens[1] = timestamp, tokens[2] = fuel
    if (tokens.size() < 3) {
        fileStream.close();
        return false;
    }

    std::string timestampRaw = trim(tokens[1]);
    std::string fuelStr      = trim(tokens[2]);

    std::string timestamp = parseTimestamp(timestampRaw);
    if (timestamp.empty()) {
        fileStream.close();
        return false;
    }

    double fuelRemaining = 0.0;
    try {
        fuelRemaining = std::stod(fuelStr);
    } catch (...) {
        fileStream.close();
        return false;
    }

    bufferedHeader = TelemetryDataPoint{timestamp, fuelRemaining};
    hasBuffered    = true;
    headerParsed   = true;

    return true;
}

std::optional<TelemetryDataPoint> TelemetryFileReader::readNext() {
    if (hasBuffered) {
        hasBuffered = false;
        return bufferedHeader;
    }

    if (!fileStream.is_open() || fileStream.eof()) return std::nullopt;

    std::string line;
    while (std::getline(fileStream, line)) {
        // Strip trailing CR
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (trim(line).empty()) continue;

        auto result = parseLine(line);
        if (result.has_value()) return result;
    }

    return std::nullopt;
}

void TelemetryFileReader::close() {
    if (fileStream.is_open()) {
        fileStream.close();
    }
}

TelemetryFileReader::~TelemetryFileReader() {
    close();
}
