#include "Logger.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

std::mutex Logger::logMutex;

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);

    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    // Format timestamp
    std::ostringstream oss;
    oss << "["
        << std::setw(4) << std::setfill('0') << (localTime.tm_year + 1900) << "-"
        << std::setw(2) << std::setfill('0') << (localTime.tm_mon + 1) << "-"
        << std::setw(2) << std::setfill('0') << localTime.tm_mday << " "
        << std::setw(2) << std::setfill('0') << localTime.tm_hour << ":"
        << std::setw(2) << std::setfill('0') << localTime.tm_min << ":"
        << std::setw(2) << std::setfill('0') << localTime.tm_sec
        << "] " << message;

    std::string logLine = oss.str();

    // Write to stdout
    std::cout << logLine << std::endl;

    // Append to log file (opened once for the lifetime of the process)
    static std::ofstream logFile("server_log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << logLine << "\n";
        logFile.flush();
    }
}
