#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <iostream>
#include <functional>
#include "../common/protocol.hpp"

#ifdef WITH_POSTGRESQL
#include "db/PostgreSQLConnection.h"
#include "db/IResultSet.h"
#endif

namespace asio_sql {

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket)
        : socket_(std::move(socket)) {
    }

    void start() {
        read_header();
    }

private:
    void read_header() {
        auto self(shared_from_this());
        
        header_buffer_.resize(MessageHeader::HEADER_SIZE);
        
        boost::asio::async_read(socket_,
            boost::asio::buffer(header_buffer_),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    try {
                        MessageHeader header = MessageHeader::deserialize(header_buffer_);
                        read_payload(header);
                    } catch (const std::exception& e) {
                        std::cerr << "Error deserializing header: " << e.what() << std::endl;
                        send_error("Invalid message header");
                    }
                } else if (ec != boost::asio::error::eof) {
                    std::cerr << "Read header error: " << ec.message() << std::endl;
                }
            });
    }

    void read_payload(const MessageHeader& header) {
        auto self(shared_from_this());
        
        payload_buffer_.resize(header.payload_size);
        
        boost::asio::async_read(socket_,
            boost::asio::buffer(payload_buffer_),
            [this, self, header](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    handle_request(header);
                } else {
                    std::cerr << "Read payload error: " << ec.message() << std::endl;
                }
            });
    }

    void handle_request(const MessageHeader& header) {
        try {
            MessageType msg_type = static_cast<MessageType>(header.message_type);
            
            std::string payload_str(payload_buffer_.begin(), payload_buffer_.end());
            
            switch (msg_type) {
                case MessageType::QUERY_RAW:
                    handle_query_raw(payload_str);
                    break;
                case MessageType::QUERY_JSON:
                    handle_query_json(payload_str);
                    break;
                case MessageType::QUERY_BINARY:
                    handle_query_binary(payload_str);
                    break;
                case MessageType::QUERY_STREAM:
                    handle_query_stream(payload_str);
                    break;
                default:
                    send_error("Unknown message type");
                    break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error handling request: " << e.what() << std::endl;
            send_error(std::string("Request handling error: ") + e.what());
        }
    }

    void handle_query_raw(const std::string& payload) {
        std::cout << "Handling QUERY_RAW request" << std::endl;
        
        QueryRequest req = QueryRequest::deserialize(payload);
        std::cout << "SQL: " << req.sql << std::endl;
        
        try {
#ifdef WITH_POSTGRESQL
            // Execute query using PostgreSQL
            if (!db_connection_) {
                // Create connection (in real app, use connection pool)
                db_connection_ = std::make_shared<PostgreSQLConnection>(
                    "host=localhost dbname=testdb user=postgres password=postgres"
                );
                db_connection_->connect();
            }
            
            auto stmt = db_connection_->createStatement(req.sql);
            auto result = stmt->executeQuery();
            
            RawRowResponse response;
            
            // Get column names
            int col_count = result->getColumnCount();
            for (int i = 0; i < col_count; ++i) {
                response.column_names.push_back(result->getColumnName(i));
            }
            
            // Get rows
            while (result->next()) {
                std::vector<std::string> row;
                for (int i = 0; i < col_count; ++i) {
                    if (result->isNull(i)) {
                        row.push_back("");
                    } else {
                        row.push_back(result->getString(i));
                    }
                }
                response.rows.push_back(row);
            }
            
            std::cout << "Query returned " << response.rows.size() << " rows" << std::endl;
            
            send_response(MessageType::RESPONSE_RAW, response.serialize());
#else
            // Mock response when PostgreSQL is not available
            RawRowResponse response;
            response.column_names = {"id", "name", "value"};
            response.rows = {
                {"1", "test1", "100"},
                {"2", "test2", "200"},
                {"3", "test3", "300"}
            };
            send_response(MessageType::RESPONSE_RAW, response.serialize());
#endif
        } catch (const std::exception& e) {
            std::cerr << "Query execution error: " << e.what() << std::endl;
            send_error(std::string("Query error: ") + e.what());
        }
    }

    void handle_query_json(const std::string& payload) {
        std::cout << "Handling QUERY_JSON request" << std::endl;
        
        QueryRequest req = QueryRequest::deserialize(payload);
        std::cout << "SQL: " << req.sql << std::endl;
        
        try {
#ifdef WITH_POSTGRESQL
            if (!db_connection_) {
                db_connection_ = std::make_shared<PostgreSQLConnection>(
                    "host=localhost dbname=testdb user=postgres password=postgres"
                );
                db_connection_->connect();
            }
            
            auto stmt = db_connection_->createStatement(req.sql);
            auto result = stmt->executeQuery();
            
            nlohmann::json json_array = nlohmann::json::array();
            
            int col_count = result->getColumnCount();
            
            while (result->next()) {
                nlohmann::json row_obj;
                for (int i = 0; i < col_count; ++i) {
                    std::string col_name = result->getColumnName(i);
                    if (result->isNull(i)) {
                        row_obj[col_name] = nullptr;
                    } else {
                        row_obj[col_name] = result->getString(i);
                    }
                }
                json_array.push_back(row_obj);
            }
            
            JsonResponse response;
            response.data = json_array;
            
            std::cout << "Query returned " << json_array.size() << " rows as JSON" << std::endl;
            
            send_response(MessageType::RESPONSE_JSON, response.serialize());
#else
            // Mock JSON response
            nlohmann::json json_array = nlohmann::json::array();
            json_array.push_back({{"id", 1}, {"name", "test1"}, {"value", 100}});
            json_array.push_back({{"id", 2}, {"name", "test2"}, {"value", 200}});
            
            JsonResponse response;
            response.data = json_array;
            send_response(MessageType::RESPONSE_JSON, response.serialize());
#endif
        } catch (const std::exception& e) {
            std::cerr << "Query execution error: " << e.what() << std::endl;
            send_error(std::string("Query error: ") + e.what());
        }
    }

    void handle_query_binary(const std::string& payload) {
        std::cout << "Handling QUERY_BINARY request" << std::endl;
        
        QueryRequest req = QueryRequest::deserialize(payload);
        std::cout << "SQL: " << req.sql << std::endl;
        
        // Example: Binary serialization of query results
        // In a real implementation, you would use a proper binary format
        // like Protocol Buffers, MessagePack, or custom binary format
        
        try {
            // For this example, we'll create a simple binary format:
            // [num_rows: 4 bytes][num_cols: 4 bytes]
            // For each row: [col1_len: 4 bytes][col1_data][col2_len: 4 bytes][col2_data]...
            
            std::vector<uint8_t> binary_data;
            
#ifdef WITH_POSTGRESQL
            if (!db_connection_) {
                db_connection_ = std::make_shared<PostgreSQLConnection>(
                    "host=localhost dbname=testdb user=postgres password=postgres"
                );
                db_connection_->connect();
            }
            
            auto stmt = db_connection_->createStatement(req.sql);
            auto result = stmt->executeQuery();
            
            int col_count = result->getColumnCount();
            std::vector<std::vector<std::string>> rows;
            
            while (result->next()) {
                std::vector<std::string> row;
                for (int i = 0; i < col_count; ++i) {
                    row.push_back(result->isNull(i) ? "" : result->getString(i));
                }
                rows.push_back(row);
            }
            
            // Serialize to binary
            uint32_t num_rows = static_cast<uint32_t>(rows.size());
            uint32_t num_cols = static_cast<uint32_t>(col_count);
            
            // Write num_rows (big-endian)
            binary_data.push_back((num_rows >> 24) & 0xFF);
            binary_data.push_back((num_rows >> 16) & 0xFF);
            binary_data.push_back((num_rows >> 8) & 0xFF);
            binary_data.push_back(num_rows & 0xFF);
            
            // Write num_cols
            binary_data.push_back((num_cols >> 24) & 0xFF);
            binary_data.push_back((num_cols >> 16) & 0xFF);
            binary_data.push_back((num_cols >> 8) & 0xFF);
            binary_data.push_back(num_cols & 0xFF);
            
            // Write each cell
            for (const auto& row : rows) {
                for (const auto& cell : row) {
                    uint32_t len = static_cast<uint32_t>(cell.size());
                    binary_data.push_back((len >> 24) & 0xFF);
                    binary_data.push_back((len >> 16) & 0xFF);
                    binary_data.push_back((len >> 8) & 0xFF);
                    binary_data.push_back(len & 0xFF);
                    binary_data.insert(binary_data.end(), cell.begin(), cell.end());
                }
            }
            
            std::cout << "Binary response size: " << binary_data.size() << " bytes" << std::endl;
#else
            // Mock binary data
            uint32_t num_rows = 2;
            uint32_t num_cols = 3;
            
            binary_data.push_back((num_rows >> 24) & 0xFF);
            binary_data.push_back((num_rows >> 16) & 0xFF);
            binary_data.push_back((num_rows >> 8) & 0xFF);
            binary_data.push_back(num_rows & 0xFF);
            
            binary_data.push_back((num_cols >> 24) & 0xFF);
            binary_data.push_back((num_cols >> 16) & 0xFF);
            binary_data.push_back((num_cols >> 8) & 0xFF);
            binary_data.push_back(num_cols & 0xFF);
            
            // Add mock data
            std::vector<std::string> mock_data = {"1", "test1", "100", "2", "test2", "200"};
            for (const auto& cell : mock_data) {
                uint32_t len = static_cast<uint32_t>(cell.size());
                binary_data.push_back((len >> 24) & 0xFF);
                binary_data.push_back((len >> 16) & 0xFF);
                binary_data.push_back((len >> 8) & 0xFF);
                binary_data.push_back(len & 0xFF);
                binary_data.insert(binary_data.end(), cell.begin(), cell.end());
            }
#endif
            
            send_response_binary(MessageType::RESPONSE_BINARY, binary_data);
            
        } catch (const std::exception& e) {
            std::cerr << "Binary query error: " << e.what() << std::endl;
            send_error(std::string("Binary query error: ") + e.what());
        }
    }

    void handle_query_stream(const std::string& payload) {
        std::cout << "Handling QUERY_STREAM request" << std::endl;
        
        QueryRequest req = QueryRequest::deserialize(payload);
        std::cout << "SQL: " << req.sql << std::endl;
        
        // Stream example: Send rows one at a time instead of all at once
        // This demonstrates how to use streaming for large result sets
        
        try {
#ifdef WITH_POSTGRESQL
            if (!db_connection_) {
                db_connection_ = std::make_shared<PostgreSQLConnection>(
                    "host=localhost dbname=testdb user=postgres password=postgres"
                );
                db_connection_->connect();
            }
            
            auto stmt = db_connection_->createStatement(req.sql);
            auto result = stmt->executeQuery();
            
            nlohmann::json stream_response = nlohmann::json::array();
            
            int col_count = result->getColumnCount();
            
            // Get column names
            nlohmann::json columns = nlohmann::json::array();
            for (int i = 0; i < col_count; ++i) {
                columns.push_back(result->getColumnName(i));
            }
            
            // Send metadata first
            nlohmann::json metadata;
            metadata["type"] = "metadata";
            metadata["columns"] = columns;
            stream_response.push_back(metadata);
            
            // Then send each row as a separate "chunk"
            int row_count = 0;
            while (result->next()) {
                nlohmann::json row_chunk;
                row_chunk["type"] = "row";
                row_chunk["index"] = row_count++;
                
                nlohmann::json row_data = nlohmann::json::array();
                for (int i = 0; i < col_count; ++i) {
                    if (result->isNull(i)) {
                        row_data.push_back(nullptr);
                    } else {
                        row_data.push_back(result->getString(i));
                    }
                }
                row_chunk["data"] = row_data;
                stream_response.push_back(row_chunk);
            }
            
            // Send end marker
            nlohmann::json end_marker;
            end_marker["type"] = "end";
            end_marker["total_rows"] = row_count;
            stream_response.push_back(end_marker);
            
            JsonResponse response;
            response.data = stream_response;
            
            std::cout << "Streamed " << row_count << " rows" << std::endl;
            
            send_response(MessageType::RESPONSE_STREAM, response.serialize());
#else
            // Mock stream response
            nlohmann::json stream_response = nlohmann::json::array();
            
            nlohmann::json metadata;
            metadata["type"] = "metadata";
            metadata["columns"] = {"id", "name", "value"};
            stream_response.push_back(metadata);
            
            nlohmann::json row1;
            row1["type"] = "row";
            row1["index"] = 0;
            row1["data"] = {1, "test1", 100};
            stream_response.push_back(row1);
            
            nlohmann::json row2;
            row2["type"] = "row";
            row2["index"] = 1;
            row2["data"] = {2, "test2", 200};
            stream_response.push_back(row2);
            
            nlohmann::json end_marker;
            end_marker["type"] = "end";
            end_marker["total_rows"] = 2;
            stream_response.push_back(end_marker);
            
            JsonResponse response;
            response.data = stream_response;
            send_response(MessageType::RESPONSE_STREAM, response.serialize());
#endif
        } catch (const std::exception& e) {
            std::cerr << "Stream query error: " << e.what() << std::endl;
            send_error(std::string("Stream query error: ") + e.what());
        }
    }

    void send_response(MessageType type, const std::string& payload) {
        MessageHeader header;
        header.message_type = static_cast<uint8_t>(type);
        header.payload_size = static_cast<uint32_t>(payload.size());
        
        std::vector<uint8_t> header_bytes = header.serialize();
        
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(header_bytes));
        buffers.push_back(boost::asio::buffer(payload));
        
        auto self(shared_from_this());
        boost::asio::async_write(socket_, buffers,
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    // Continue reading next request
                    read_header();
                } else {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                }
            });
    }

    void send_response_binary(MessageType type, const std::vector<uint8_t>& payload) {
        MessageHeader header;
        header.message_type = static_cast<uint8_t>(type);
        header.payload_size = static_cast<uint32_t>(payload.size());
        
        std::vector<uint8_t> header_bytes = header.serialize();
        
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(header_bytes));
        buffers.push_back(boost::asio::buffer(payload));
        
        auto self(shared_from_this());
        boost::asio::async_write(socket_, buffers,
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    read_header();
                } else {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                }
            });
    }

    void send_error(const std::string& error_msg) {
        ErrorResponse error;
        error.error_message = error_msg;
        send_response(MessageType::RESPONSE_ERROR, error.serialize());
    }

    tcp::socket socket_;
    std::vector<uint8_t> header_buffer_;
    std::vector<uint8_t> payload_buffer_;
    
#ifdef WITH_POSTGRESQL
    std::shared_ptr<PostgreSQLConnection> db_connection_;
#endif
};

class SqlServer {
public:
    SqlServer(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "Client connected from " 
                              << socket.remote_endpoint() << std::endl;
                    std::make_shared<Session>(std::move(socket))->start();
                }
                
                accept();
            });
    }

    tcp::acceptor acceptor_;
};

} // namespace asio_sql
