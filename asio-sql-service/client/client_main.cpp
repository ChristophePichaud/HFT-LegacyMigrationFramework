#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "sql_client.hpp"

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [host] [port]" << std::endl;
    std::cout << "  host: Server hostname or IP (default: localhost)" << std::endl;
    std::cout << "  port: Server port number (default: 9090)" << std::endl;
}

void print_raw_response(const asio_sql::RawRowResponse& response) {
    std::cout << "\n--- Raw Response ---" << std::endl;
    
    // Print column headers
    for (size_t i = 0; i < response.column_names.size(); ++i) {
        std::cout << response.column_names[i];
        if (i < response.column_names.size() - 1) {
            std::cout << " | ";
        }
    }
    std::cout << std::endl;
    
    // Print separator
    for (size_t i = 0; i < response.column_names.size(); ++i) {
        std::cout << std::string(response.column_names[i].length(), '-');
        if (i < response.column_names.size() - 1) {
            std::cout << "-+-";
        }
    }
    std::cout << std::endl;
    
    // Print rows
    for (const auto& row : response.rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << row[i];
            if (i < row.size() - 1) {
                std::cout << " | ";
            }
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nTotal rows: " << response.rows.size() << std::endl;
}

void print_json_response(const asio_sql::JsonResponse& response) {
    std::cout << "\n--- JSON Response ---" << std::endl;
    std::cout << response.data.dump(2) << std::endl;
}

void print_binary_response(const asio_sql::BinaryResponse& response) {
    std::cout << "\n--- Binary Response ---" << std::endl;
    std::cout << "Binary data size: " << response.data.size() << " bytes" << std::endl;
    
    // Parse the binary format
    if (response.data.size() < 8) {
        std::cout << "Invalid binary data" << std::endl;
        return;
    }
    
    const uint8_t* data = response.data.data();
    size_t offset = 0;
    
    // Read num_rows
    uint32_t num_rows = (static_cast<uint32_t>(data[offset]) << 24) |
                       (static_cast<uint32_t>(data[offset + 1]) << 16) |
                       (static_cast<uint32_t>(data[offset + 2]) << 8) |
                       static_cast<uint32_t>(data[offset + 3]);
    offset += 4;
    
    // Read num_cols
    uint32_t num_cols = (static_cast<uint32_t>(data[offset]) << 24) |
                       (static_cast<uint32_t>(data[offset + 1]) << 16) |
                       (static_cast<uint32_t>(data[offset + 2]) << 8) |
                       static_cast<uint32_t>(data[offset + 3]);
    offset += 4;
    
    std::cout << "Rows: " << num_rows << ", Columns: " << num_cols << std::endl;
    
    // Read cells
    for (uint32_t row = 0; row < num_rows; ++row) {
        std::cout << "Row " << row << ": ";
        for (uint32_t col = 0; col < num_cols; ++col) {
            if (offset + 4 > response.data.size()) {
                std::cout << "\nError: Unexpected end of data" << std::endl;
                return;
            }
            
            uint32_t len = (static_cast<uint32_t>(data[offset]) << 24) |
                          (static_cast<uint32_t>(data[offset + 1]) << 16) |
                          (static_cast<uint32_t>(data[offset + 2]) << 8) |
                          static_cast<uint32_t>(data[offset + 3]);
            offset += 4;
            
            if (offset + len > response.data.size()) {
                std::cout << "\nError: Cell data out of bounds" << std::endl;
                return;
            }
            
            std::string cell_value(data + offset, data + offset + len);
            offset += len;
            
            std::cout << cell_value;
            if (col < num_cols - 1) {
                std::cout << " | ";
            }
        }
        std::cout << std::endl;
    }
}

void print_stream_response(const asio_sql::JsonResponse& response) {
    std::cout << "\n--- Stream Response ---" << std::endl;
    
    if (!response.data.is_array()) {
        std::cout << "Invalid stream response" << std::endl;
        return;
    }
    
    for (const auto& chunk : response.data) {
        std::string type = chunk["type"];
        
        if (type == "metadata") {
            std::cout << "Metadata received:" << std::endl;
            std::cout << "  Columns: ";
            for (const auto& col : chunk["columns"]) {
                std::cout << col << " ";
            }
            std::cout << std::endl;
        } else if (type == "row") {
            int index = chunk["index"];
            std::cout << "Row " << index << ": ";
            bool first = true;
            for (const auto& val : chunk["data"]) {
                if (!first) std::cout << " | ";
                first = false;
                if (val.is_null()) {
                    std::cout << "NULL";
                } else {
                    std::cout << val;
                }
            }
            std::cout << std::endl;
        } else if (type == "end") {
            int total = chunk["total_rows"];
            std::cout << "Stream ended. Total rows: " << total << std::endl;
        }
    }
}

void run_interactive_mode(asio_sql::SqlClient& client) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "        Interactive SQL Client Mode               " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  1) raw <sql>    - Execute query and get raw rows" << std::endl;
    std::cout << "  2) json <sql>   - Execute query and get JSON" << std::endl;
    std::cout << "  3) binary <sql> - Execute query and get binary data" << std::endl;
    std::cout << "  4) stream <sql> - Execute query and get streamed results" << std::endl;
    std::cout << "  5) quit         - Exit the client" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << std::endl;
    
    std::string line;
    while (true) {
        std::cout << "sql> ";
        std::getline(std::cin, line);
        
        if (line.empty()) {
            continue;
        }
        
        // Parse command
        size_t space_pos = line.find(' ');
        std::string command = line.substr(0, space_pos);
        std::string sql = (space_pos != std::string::npos) ? line.substr(space_pos + 1) : "";
        
        if (command == "quit" || command == "exit" || command == "q") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        
        try {
            if (command == "raw") {
                if (sql.empty()) {
                    std::cout << "Usage: raw <sql>" << std::endl;
                    continue;
                }
                auto response = client.query_raw(sql);
                print_raw_response(response);
                
            } else if (command == "json") {
                if (sql.empty()) {
                    std::cout << "Usage: json <sql>" << std::endl;
                    continue;
                }
                auto response = client.query_json(sql);
                print_json_response(response);
                
            } else if (command == "binary") {
                if (sql.empty()) {
                    std::cout << "Usage: binary <sql>" << std::endl;
                    continue;
                }
                auto response = client.query_binary(sql);
                print_binary_response(response);
                
            } else if (command == "stream") {
                if (sql.empty()) {
                    std::cout << "Usage: stream <sql>" << std::endl;
                    continue;
                }
                auto response = client.query_stream(sql);
                print_stream_response(response);
                
            } else {
                std::cout << "Unknown command: " << command << std::endl;
                std::cout << "Type 'quit' to exit or use one of: raw, json, binary, stream" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        std::string host = "localhost";
        std::string port = "9090";
        
        if (argc > 1) {
            if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
                print_usage(argv[0]);
                return 0;
            }
            host = argv[1];
        }
        
        if (argc > 2) {
            port = argv[2];
        }
        
        std::cout << "==================================================" << std::endl;
        std::cout << "     HFT SQL Service Client (Boost.Asio 1.89)    " << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << std::endl;
        
        std::cout << "Connecting to " << host << ":" << port << "..." << std::endl;
        
        boost::asio::io_context io_context;
        asio_sql::SqlClient client(io_context);
        
        client.connect(host, port);
        
        std::cout << "Connected successfully!" << std::endl;
        
        // Run interactive mode
        run_interactive_mode(client);
        
        client.disconnect();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
