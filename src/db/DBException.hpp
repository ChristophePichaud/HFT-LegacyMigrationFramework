#pragma once
#include <stdexcept>
#include <string>

enum class DBErrorCode {
    UNKNOWN = 0,
    CONNECTION_FAILED = 1,
    QUERY_FAILED = 2,
    TRANSACTION_FAILED = 3,
    PARAMETER_BINDING_FAILED = 4,
    RESULT_PROCESSING_FAILED = 5,
    NOT_IMPLEMENTED = 6,
    INVALID_PARAMETER = 7,
    RESOURCE_NOT_FOUND = 8,
    CONSTRAINT_VIOLATION = 9,
    TIMEOUT = 10
};

class DBException : public std::runtime_error {
public:
    explicit DBException(const std::string& msg)
        : std::runtime_error(msg), _errorCode(DBErrorCode::UNKNOWN) {}
    
    DBException(DBErrorCode code, const std::string& msg)
        : std::runtime_error(msg), _errorCode(code) {}
    
    DBException(DBErrorCode code, const std::string& msg, const std::string& context)
        : std::runtime_error(msg + " [Context: " + context + "]"), 
          _errorCode(code), _context(context) {}
    
    DBErrorCode getErrorCode() const { return _errorCode; }
    const std::string& getContext() const { return _context; }
    
    static std::string errorCodeToString(DBErrorCode code) {
        switch (code) {
            case DBErrorCode::UNKNOWN: return "UNKNOWN";
            case DBErrorCode::CONNECTION_FAILED: return "CONNECTION_FAILED";
            case DBErrorCode::QUERY_FAILED: return "QUERY_FAILED";
            case DBErrorCode::TRANSACTION_FAILED: return "TRANSACTION_FAILED";
            case DBErrorCode::PARAMETER_BINDING_FAILED: return "PARAMETER_BINDING_FAILED";
            case DBErrorCode::RESULT_PROCESSING_FAILED: return "RESULT_PROCESSING_FAILED";
            case DBErrorCode::NOT_IMPLEMENTED: return "NOT_IMPLEMENTED";
            case DBErrorCode::INVALID_PARAMETER: return "INVALID_PARAMETER";
            case DBErrorCode::RESOURCE_NOT_FOUND: return "RESOURCE_NOT_FOUND";
            case DBErrorCode::CONSTRAINT_VIOLATION: return "CONSTRAINT_VIOLATION";
            case DBErrorCode::TIMEOUT: return "TIMEOUT";
            default: return "UNKNOWN";
        }
    }
    
private:
    DBErrorCode _errorCode;
    std::string _context;
};
