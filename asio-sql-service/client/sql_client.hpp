#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "../common/protocol.hpp"

namespace asio_sql {

using boost::asio::ip::tcp;

class SqlClient {
public:
    SqlClient(boost::asio::io_context& io_context)
        : socket_(io_context) {
    }

    void connect(const std::string& host, const std::string& port) {
        tcp::resolver resolver(socket_.get_executor());
        auto endpoints = resolver.resolve(host, port);
        boost::asio::connect(socket_, endpoints);
        std::cout << "Connected to " << host << ":" << port << std::endl;
    }

    void disconnect() {
        if (socket_.is_open()) {
            socket_.close();
        }
    }

    RawRowResponse query_raw(const std::string& sql) {
        QueryRequest req;
        req.sql = sql;
        
        send_request(MessageType::QUERY_RAW, req.serialize());
        
        auto [header, payload] = receive_response();
        
        if (static_cast<MessageType>(header.message_type) == MessageType::RESPONSE_ERROR) {
            ErrorResponse error = ErrorResponse::deserialize(
                std::string(payload.begin(), payload.end())
            );
            throw std::runtime_error("Server error: " + error.error_message);
        }
        
        if (static_cast<MessageType>(header.message_type) != MessageType::RESPONSE_RAW) {
            throw std::runtime_error("Unexpected response type");
        }
        
        std::string payload_str(payload.begin(), payload.end());
        return RawRowResponse::deserialize(payload_str);
    }

    JsonResponse query_json(const std::string& sql) {
        QueryRequest req;
        req.sql = sql;
        
        send_request(MessageType::QUERY_JSON, req.serialize());
        
        auto [header, payload] = receive_response();
        
        if (static_cast<MessageType>(header.message_type) == MessageType::RESPONSE_ERROR) {
            ErrorResponse error = ErrorResponse::deserialize(
                std::string(payload.begin(), payload.end())
            );
            throw std::runtime_error("Server error: " + error.error_message);
        }
        
        if (static_cast<MessageType>(header.message_type) != MessageType::RESPONSE_JSON) {
            throw std::runtime_error("Unexpected response type");
        }
        
        std::string payload_str(payload.begin(), payload.end());
        return JsonResponse::deserialize(payload_str);
    }

    BinaryResponse query_binary(const std::string& sql) {
        QueryRequest req;
        req.sql = sql;
        
        send_request(MessageType::QUERY_BINARY, req.serialize());
        
        auto [header, payload] = receive_response();
        
        if (static_cast<MessageType>(header.message_type) == MessageType::RESPONSE_ERROR) {
            ErrorResponse error = ErrorResponse::deserialize(
                std::string(payload.begin(), payload.end())
            );
            throw std::runtime_error("Server error: " + error.error_message);
        }
        
        if (static_cast<MessageType>(header.message_type) != MessageType::RESPONSE_BINARY) {
            throw std::runtime_error("Unexpected response type");
        }
        
        return BinaryResponse::deserialize(payload);
    }

    JsonResponse query_stream(const std::string& sql) {
        QueryRequest req;
        req.sql = sql;
        
        send_request(MessageType::QUERY_STREAM, req.serialize());
        
        auto [header, payload] = receive_response();
        
        if (static_cast<MessageType>(header.message_type) == MessageType::RESPONSE_ERROR) {
            ErrorResponse error = ErrorResponse::deserialize(
                std::string(payload.begin(), payload.end())
            );
            throw std::runtime_error("Server error: " + error.error_message);
        }
        
        if (static_cast<MessageType>(header.message_type) != MessageType::RESPONSE_STREAM) {
            throw std::runtime_error("Unexpected response type");
        }
        
        std::string payload_str(payload.begin(), payload.end());
        return JsonResponse::deserialize(payload_str);
    }

private:
    void send_request(MessageType type, const std::string& payload) {
        MessageHeader header;
        header.message_type = static_cast<uint8_t>(type);
        header.payload_size = static_cast<uint32_t>(payload.size());
        
        std::vector<uint8_t> header_bytes = header.serialize();
        
        // Send header
        boost::asio::write(socket_, boost::asio::buffer(header_bytes));
        
        // Send payload
        boost::asio::write(socket_, boost::asio::buffer(payload));
    }

    std::pair<MessageHeader, std::vector<uint8_t>> receive_response() {
        // Read header
        std::vector<uint8_t> header_buffer(MessageHeader::HEADER_SIZE);
        boost::asio::read(socket_, boost::asio::buffer(header_buffer));
        
        MessageHeader header = MessageHeader::deserialize(header_buffer);
        
        // Read payload
        std::vector<uint8_t> payload(header.payload_size);
        if (header.payload_size > 0) {
            boost::asio::read(socket_, boost::asio::buffer(payload));
        }
        
        return {header, payload};
    }

    tcp::socket socket_;
};

} // namespace asio_sql
