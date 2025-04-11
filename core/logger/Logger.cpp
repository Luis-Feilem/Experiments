#include "Logger.hpp"

// Constructor
Logger::Logger(Logger::LogLevel log_level) : log_level(log_level) {}

// Destructor
Logger::~Logger() {}

// Set the log level
void Logger::set_level(Logger::LogLevel level) {
    log_level = level;
}

// Helper function to convert LogLevel to string
std::string Logger::level_to_string(Logger::LogLevel level) {
    switch (level) {
        case Logger::LogLevel::DEBUG: return "DEBUG";
        case Logger::LogLevel::INFO: return "INFO";
        case Logger::LogLevel::STUDY: return "STUDY";
        case Logger::LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Helper function to convert string to LogLevel
Logger::LogLevel Logger::string_to_level(const std::string& level) {
    if (level.empty()) {
        std::cerr <<"Missing log level: " << level << ", defaulting to INFO" << std::endl;
        return Logger::LogLevel::INFO;
    }

    if (level == "DEBUG") {
        return Logger::LogLevel::DEBUG;
    } else if (level == "INFO") {
        return Logger::LogLevel::INFO;
    } else if (level == "STUDY") {
        return Logger::LogLevel::STUDY;
    } else if (level == "ERROR") {
        return Logger::LogLevel::ERROR;
    } else {
        std::cerr <<"Invalid log level: " << level << ", defaulting to INFO" << std::endl;
        return Logger::LogLevel::INFO;
    }
}


// Get log level
Logger::LogLevel Logger::get_level(){
    return log_level;
}

// Log debug messages
void Logger::log_debug(const std::string& message) {
    if (log_level <= Logger::LogLevel::DEBUG) {
        std::cout << "[" << level_to_string(Logger::LogLevel::DEBUG) << "] " << message << std::endl << std::flush;
    }
}

// Log info messages
void Logger::log_info(const std::string& message) {
    if (log_level <= Logger::LogLevel::INFO) {
        std::cout << "[" << level_to_string(Logger::LogLevel::INFO) << "] " << message << std::endl << std::flush;
    }
}

// Log study messages
void Logger::log_study(const std::string& message) {
    if (log_level <= Logger::LogLevel::STUDY) {
        std::cout << "[" << level_to_string(Logger::LogLevel::STUDY) << "] " << message << std::endl << std::flush;
    }
}

// Log error messages
void Logger::log_error(const std::string& message) {
    if (log_level <= Logger::LogLevel::ERROR) {
        std::cout << "[" << level_to_string(Logger::LogLevel::ERROR) << "] " << message << std::endl << std::flush;
    }
}
