#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace asio_sql {

// Message types
enum class MessageType : uint8_t {
    QUERY_RAW = 1,           // Execute query, return raw rows
    QUERY_JSON = 2,          // Execute query, return JSON
    QUERY_BINARY = 3,        // Execute query, return binary serialized data
    QUERY_STREAM = 4,        // Execute query, stream results
    RESPONSE_RAW = 11,       // Response with raw row data
    RESPONSE_JSON = 12,      // Response with JSON data
    RESPONSE_BINARY = 13,    // Response with binary data
    RESPONSE_STREAM = 14,    // Response with streamed data
    RESPONSE_ERROR = 99      // Error response
};

// Message header (fixed size)
struct MessageHeader {
    uint8_t message_type;    // MessageType enum
    uint32_t payload_size;   // Size of payload in bytes
    
    static constexpr size_t HEADER_SIZE = sizeof(uint8_t) + sizeof(uint32_t);
    
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer;
        buffer.reserve(HEADER_SIZE);
        buffer.push_back(message_type);
        
        // Serialize payload_size (big-endian)
        buffer.push_back((payload_size >> 24) & 0xFF);
        buffer.push_back((payload_size >> 16) & 0xFF);
        buffer.push_back((payload_size >> 8) & 0xFF);
        buffer.push_back(payload_size & 0xFF);
        
        return buffer;
    }
    
    static MessageHeader deserialize(const std::vector<uint8_t>& buffer) {
        if (buffer.size() < HEADER_SIZE) {
            throw std::runtime_error("Buffer too small for header");
        }
        
        MessageHeader header;
        header.message_type = buffer[0];
        header.payload_size = (static_cast<uint32_t>(buffer[1]) << 24) |
                             (static_cast<uint32_t>(buffer[2]) << 16) |
                             (static_cast<uint32_t>(buffer[3]) << 8) |
                             static_cast<uint32_t>(buffer[4]);
        return header;
    }
};

// Request message for SQL query
struct QueryRequest {
    std::string sql;
    
    std::string serialize() const {
        return sql;
    }
    
    static QueryRequest deserialize(const std::string& data) {
        QueryRequest req;
        req.sql = data;
        return req;
    }
};

// Response with raw row data (column name/value pairs)
struct RawRowResponse {
    std::vector<std::string> column_names;
    std::vector<std::vector<std::string>> rows;  // Each row is vector of string values
    
    std::string serialize() const {
        nlohmann::json j;
        j["columns"] = column_names;
        j["rows"] = rows;
        return j.dump();
    }
    
    static RawRowResponse deserialize(const std::string& data) {
        nlohmann::json j = nlohmann::json::parse(data);
        RawRowResponse resp;
        resp.column_names = j["columns"].get<std::vector<std::string>>();
        resp.rows = j["rows"].get<std::vector<std::vector<std::string>>>();
        return resp;
    }
};

// Response with JSON data
struct JsonResponse {
    nlohmann::json data;
    
    std::string serialize() const {
        return data.dump();
    }
    
    static JsonResponse deserialize(const std::string& data_str) {
        JsonResponse resp;
        resp.data = nlohmann::json::parse(data_str);
        return resp;
    }
};

// Binary serialized data response
struct BinaryResponse {
    std::vector<uint8_t> data;
    
    std::vector<uint8_t> serialize() const {
        return data;
    }
    
    static BinaryResponse deserialize(const std::vector<uint8_t>& data) {
        BinaryResponse resp;
        resp.data = data;
        return resp;
    }
};

// Error response
struct ErrorResponse {
    std::string error_message;
    
    std::string serialize() const {
        nlohmann::json j;
        j["error"] = error_message;
        return j.dump();
    }
    
    static ErrorResponse deserialize(const std::string& data) {
        nlohmann::json j = nlohmann::json::parse(data);
        ErrorResponse resp;
        resp.error_message = j["error"].get<std::string>();
        return resp;
    }
};

} // namespace asio_sql
