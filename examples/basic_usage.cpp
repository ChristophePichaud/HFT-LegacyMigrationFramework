#include "hft/db/PostgreSQLConnection.h"
#include <iostream>

int main() {
    std::cout << "=== HFT Legacy Migration Framework - Basic Usage ===" << std::endl;
    
    // Create connection
    auto conn = std::make_shared<hft::db::PostgreSQLConnection>();
    
    // Open connection
    std::cout << "Connecting to PostgreSQL..." << std::endl;
    if (!conn->open("host=localhost port=5432 dbname=test user=postgres password=postgres")) {
        std::cerr << "Failed to connect: " << conn->getLastError() << std::endl;
        return 1;
    }
    
    std::cout << "Connected successfully!" << std::endl;
    
    // Execute simple query
    std::cout << "Executing simple query..." << std::endl;
    if (conn->execute("SELECT version()")) {
        std::cout << "Query executed successfully!" << std::endl;
    }
    
    // Create a prepared statement
    std::cout << "Testing prepared statement..." << std::endl;
    auto stmt = conn->createStatement("SELECT $1::int + $2::int AS sum");
    stmt->bindInt(1, 10);
    stmt->bindInt(2, 20);
    
    auto result = stmt->executeQuery();
    if (result && result->next()) {
        std::cout << "10 + 20 = " << result->getInt(0) << std::endl;
    }
    
    // Test transaction
    std::cout << "Testing transaction..." << std::endl;
    auto txn = conn->beginTransaction();
    
    conn->execute("SELECT 1");
    
    if (txn->commit()) {
        std::cout << "Transaction committed successfully!" << std::endl;
    }
    
    // Close connection
    conn->close();
    std::cout << "Connection closed." << std::endl;
    
    return 0;
}
