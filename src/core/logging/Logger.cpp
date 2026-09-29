#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace ssa::core::logging {

void Logger::init() {
    // Basic initialization for now
    info("Logger initialized.");
}

void Logger::log(LogLevel level, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");

    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG:   levelStr = "DEBUG"; break;
        case LogLevel::INFO:    levelStr = "INFO "; break;
        case LogLevel::WARNING: levelStr = "WARN "; break;
        case LogLevel::ERROR:   levelStr = "ERROR"; break;
    }
    
    std::string formatted = "[" + ss.str() + "] [" + levelStr + "] " + message;
    std::cout << formatted << std::endl;
}

void Logger::debug(const std::string& message) { log(LogLevel::DEBUG, message); }
void Logger::info(const std::string& message) { log(LogLevel::INFO, message); }
void Logger::warning(const std::string& message) { log(LogLevel::WARNING, message); }
void Logger::error(const std::string& message) { log(LogLevel::ERROR, message); }

} // namespace ssa::core::logging
