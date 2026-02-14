#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "sql_server.hpp"

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [port]" << std::endl;
    std::cout << "  port: Port number to listen on (default: 9090)" << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        int port = 9090;
        
        if (argc > 1) {
            if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
                print_usage(argv[0]);
                return 0;
            }
            try {
                port = std::stoi(argv[1]);
                if (port < 1 || port > 65535) {
                    std::cerr << "Error: Port must be between 1 and 65535" << std::endl;
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid port number" << std::endl;
                return 1;
            }
        }
        
        std::cout << "==================================================" << std::endl;
        std::cout << "     HFT SQL Service Server (Boost.Asio 1.89)    " << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << std::endl;
        
        std::cout << "Starting server on port " << port << "..." << std::endl;
        
#ifdef WITH_POSTGRESQL
        std::cout << "PostgreSQL support: ENABLED" << std::endl;
#else
        std::cout << "PostgreSQL support: DISABLED (using mock data)" << std::endl;
#endif
        
#ifdef WITH_SYBASE
        std::cout << "Sybase support: ENABLED" << std::endl;
#else
        std::cout << "Sybase support: DISABLED" << std::endl;
#endif
        
        std::cout << std::endl;
        std::cout << "Supported query types:" << std::endl;
        std::cout << "  - QUERY_RAW: Returns raw rows with column names" << std::endl;
        std::cout << "  - QUERY_JSON: Returns results as JSON array" << std::endl;
        std::cout << "  - QUERY_BINARY: Returns binary serialized data" << std::endl;
        std::cout << "  - QUERY_STREAM: Returns streamed results with metadata" << std::endl;
        std::cout << std::endl;
        
        boost::asio::io_context io_context;
        
        asio_sql::SqlServer server(io_context, port);
        
        std::cout << "Server listening on port " << port << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        std::cout << std::endl;
        
        io_context.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
