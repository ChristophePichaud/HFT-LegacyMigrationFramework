#include <iostream>
#include <boost/asio.hpp>
#include "../client/sql_client.hpp"

// Example: Using the client to query data and work with HFT entities
// This demonstrates how the client integrates with the existing HFT framework

int main() {
    try {
        std::cout << "=== HFT Asio SQL Service - Integration Example ===" << std::endl;
        std::cout << std::endl;
        
        // Create Boost.Asio I/O context
        boost::asio::io_context io_context;
        
        // Create SQL client
        asio_sql::SqlClient client(io_context);
        
        // Connect to server
        std::string host = "localhost";
        std::string port = "9090";
        
        std::cout << "Connecting to " << host << ":" << port << "..." << std::endl;
        client.connect(host, port);
        std::cout << "Connected!" << std::endl << std::endl;
        
        // Example 1: Query raw rows (compatible with IDBRow interface)
        std::cout << "Example 1: RAW query (IDBRow-compatible)" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        try {
            auto raw_response = client.query_raw("SELECT * FROM users LIMIT 5");
            
            std::cout << "Columns: ";
            for (const auto& col : raw_response.column_names) {
                std::cout << col << " ";
            }
            std::cout << std::endl;
            
            std::cout << "Rows retrieved: " << raw_response.rows.size() << std::endl;
            
            // Access data similar to IDBRow interface
            for (size_t row_idx = 0; row_idx < raw_response.rows.size(); ++row_idx) {
                std::cout << "  Row " << row_idx << ": ";
                for (size_t col_idx = 0; col_idx < raw_response.rows[row_idx].size(); ++col_idx) {
                    std::cout << raw_response.rows[row_idx][col_idx] << " | ";
                }
                std::cout << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        // Example 2: Query JSON (compatible with BaseEntity::toJson())
        std::cout << "Example 2: JSON query (Entity-compatible)" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        try {
            auto json_response = client.query_json("SELECT * FROM products LIMIT 3");
            
            std::cout << "JSON Response:" << std::endl;
            std::cout << json_response.data.dump(2) << std::endl;
            
            // Process as entities would
            std::cout << "\nProcessing as entities:" << std::endl;
            for (const auto& row : json_response.data) {
                if (row.contains("id") && row.contains("name")) {
                    std::cout << "  Product ID: " << row["id"] 
                              << ", Name: " << row["name"] << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        // Example 3: Binary serialization (for high-performance scenarios)
        std::cout << "Example 3: BINARY query (optimized)" << std::endl;
        std::cout << "-------------------------------------" << std::endl;
        try {
            auto binary_response = client.query_binary("SELECT * FROM orders LIMIT 10");
            
            std::cout << "Binary data size: " << binary_response.data.size() << " bytes" << std::endl;
            
            // Parse binary format
            if (binary_response.data.size() >= 8) {
                const uint8_t* data = binary_response.data.data();
                
                uint32_t num_rows = (uint32_t(data[0]) << 24) |
                                   (uint32_t(data[1]) << 16) |
                                   (uint32_t(data[2]) << 8) |
                                   uint32_t(data[3]);
                
                uint32_t num_cols = (uint32_t(data[4]) << 24) |
                                   (uint32_t(data[5]) << 16) |
                                   (uint32_t(data[6]) << 8) |
                                   uint32_t(data[7]);
                
                std::cout << "Rows: " << num_rows << ", Columns: " << num_cols << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        // Example 4: Streaming results (for large datasets)
        std::cout << "Example 4: STREAM query (large datasets)" << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        try {
            auto stream_response = client.query_stream("SELECT * FROM large_table");
            
            int row_count = 0;
            for (const auto& chunk : stream_response.data) {
                std::string type = chunk["type"];
                
                if (type == "metadata") {
                    std::cout << "Stream metadata received" << std::endl;
                    std::cout << "Columns: ";
                    for (const auto& col : chunk["columns"]) {
                        std::cout << col << " ";
                    }
                    std::cout << std::endl;
                } else if (type == "row") {
                    row_count++;
                } else if (type == "end") {
                    std::cout << "Stream completed: " << chunk["total_rows"] 
                              << " total rows" << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        // Example 5: Error handling
        std::cout << "Example 5: Error handling" << std::endl;
        std::cout << "-------------------------" << std::endl;
        try {
            // This should fail with invalid SQL
            auto response = client.query_raw("INVALID SQL QUERY");
        } catch (const std::exception& e) {
            std::cout << "Expected error caught: " << e.what() << std::endl;
        }
        std::cout << std::endl;
        
        // Disconnect
        client.disconnect();
        std::cout << "Disconnected from server." << std::endl;
        
        std::cout << std::endl;
        std::cout << "=== Integration Examples Complete ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
