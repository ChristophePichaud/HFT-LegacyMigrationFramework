#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(_mutex);
        _logLevel = level;
    }

    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_logFile.is_open()) {
            _logFile.close();
        }
        _logFile.open(filename, std::ios::app);
        _logToFile = true;
    }

    void log(LogLevel level, const std::string& message, const std::string& file = "", int line = 0) {
        if (level < _logLevel) {
            return;
        }

        std::lock_guard<std::mutex> lock(_mutex);
        
        // Format log message
        std::ostringstream oss;
        oss << getCurrentTime() << " ";
        oss << "[" << levelToString(level) << "] ";
        
        if (!file.empty()) {
            oss << file << ":" << line << " - ";
        }
        
        oss << message << "\n";
        
        std::string logMessage = oss.str();
        
        // Write to console
        if (_logToConsole) {
            if (level >= LogLevel::ERROR) {
                std::cerr << logMessage;
            } else {
                std::cout << logMessage;
            }
        }
        
        // Write to file
        if (_logToFile && _logFile.is_open()) {
            _logFile << logMessage;
            _logFile.flush();
        }
    }

    void enableConsoleLogging(bool enable) {
        std::lock_guard<std::mutex> lock(_mutex);
        _logToConsole = enable;
    }

private:
    Logger() : _logLevel(LogLevel::INFO), _logToConsole(true), _logToFile(false) {}
    ~Logger() {
        if (_logFile.is_open()) {
            _logFile.close();
        }
    }

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        // Thread-safe time conversion
        std::tm tm_buf;
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_buf, &time);
#else
        localtime_r(&time, &tm_buf);
#endif
        
        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG:   return "DEBUG";
            case LogLevel::INFO:    return "INFO ";
            case LogLevel::WARNING: return "WARN ";
            case LogLevel::ERROR:   return "ERROR";
            case LogLevel::FATAL:   return "FATAL";
            default:                return "UNKNOWN";
        }
    }

    LogLevel _logLevel;
    bool _logToConsole;
    bool _logToFile;
    std::ofstream _logFile;
    std::mutex _mutex;
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::instance().log(LogLevel::DEBUG, msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::instance().log(LogLevel::INFO, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::instance().log(LogLevel::WARNING, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::instance().log(LogLevel::ERROR, msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) Logger::instance().log(LogLevel::FATAL, msg, __FILE__, __LINE__)
