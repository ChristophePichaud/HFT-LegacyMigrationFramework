#ifdef WITH_POSTGRESQL
#include "hft/db/PostgreSQLConnection.h"
#endif

#ifdef WITH_SYBASE
#include "hft/db/SybaseConnection.h"
#endif

#include "hft/db/IConnection.h"
#include "hft/catalog/Catalog.h"
#include "hft/codegen/CodeGenerator.h"
#include "hft/reflection/EntityTraits.h"
#include "hft/orm/Repository.h"
#include "cxxopts/cxxopts.hpp"
#include "nlohmann/json.hpp"

#include <iostream>
#include <fstream>
#include <memory>
#include <iomanip>

using json = nlohmann::json;
using namespace hft::db;
using namespace hft::catalog;
using namespace hft::codegen;
using namespace hft::reflection;
using namespace hft::orm;

// Product entity for testing
struct Product {
    int64_t id = 0;
    std::string name = "";
    std::string description = "";
    double price = 0.0;
    int32_t quantity = 0;
};

// Define entity traits for Product
namespace hft { namespace reflection {
template<>
struct EntityTraits<Product> {
    static constexpr const char* tableName() { return "products"; }
    static constexpr size_t fieldCount() { return 5; }
    
    template<typename Func>
    static void forEachField(Product& entity, Func&& func) {
        func("id", getFieldType<decltype(entity.id)>(), &entity.id, true, false);
        func("name", getFieldType<decltype(entity.name)>(), &entity.name, false, false);
        func("description", getFieldType<decltype(entity.description)>(), &entity.description, false, true);
        func("price", getFieldType<decltype(entity.price)>(), &entity.price, false, false);
        func("quantity", getFieldType<decltype(entity.quantity)>(), &entity.quantity, false, false);
    }
};
}} // namespace hft::reflection

// Helper function to validate table/column names to prevent SQL injection
bool isValidIdentifier(const std::string& identifier) {
    if (identifier.empty() || identifier.length() > 128) {
        return false;
    }
    
    // Check first character (must be letter or underscore)
    if (!std::isalpha(identifier[0]) && identifier[0] != '_') {
        return false;
    }
    
    // Check remaining characters (letters, digits, underscores)
    for (size_t i = 1; i < identifier.length(); i++) {
        if (!std::isalnum(identifier[i]) && identifier[i] != '_') {
            return false;
        }
    }
    
    return true;
}

// Helper function to print separator
void printSeparator(const std::string& title = "") {
    std::cout << "\n";
    std::cout << "========================================" << std::endl;
    if (!title.empty()) {
        std::cout << "  " << title << std::endl;
        std::cout << "========================================" << std::endl;
    }
}

// Test Sybase connection
bool testSybaseConnection(const std::string& user, const std::string& password, const std::string& server) {
#ifdef WITH_SYBASE
    printSeparator("Testing Sybase Connection");
    
    try {
        auto conn = std::make_shared<SybaseConnection>();
        
        // Build connection string
        std::string connStr = "user=" + user + ";password=" + password + ";server=" + server;
        
        std::cout << "Connecting to Sybase server: " << server << std::endl;
        if (!conn->open(connStr)) {
            std::cerr << "Failed to connect: " << conn->getLastError() << std::endl;
            return false;
        }
        
        std::cout << "✓ Connection successful!" << std::endl;
        
        // Count tables
        std::cout << "\nCounting tables in database..." << std::endl;
        auto stmt = conn->createStatement("SELECT COUNT(*) as table_count FROM sysobjects WHERE type='U'");
        auto result = stmt->executeQuery();
        
        if (result && result->next()) {
            int tableCount = result->getInt(0);
            std::cout << "✓ Total tables: " << tableCount << std::endl;
        }
        
        conn->close();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
#else
    std::cerr << "Error: Sybase support not compiled in. Rebuild with -DWITH_SYBASE=ON" << std::endl;
    return false;
#endif
}

// Test PostgreSQL connection
bool testPostgreSQLConnection(const std::string& user, const std::string& password, const std::string& dbname, const std::string& host) {
#ifdef WITH_POSTGRESQL
    printSeparator("Testing PostgreSQL Connection");
    
    try {
        auto conn = std::make_shared<PostgreSQLConnection>();
        
        // Build connection string
        std::string connStr = "host=" + host + " dbname=" + dbname + " user=" + user + " password=" + password;
        
        std::cout << "Connecting to PostgreSQL database: " << dbname << " on " << host << std::endl;
        if (!conn->open(connStr)) {
            std::cerr << "Failed to connect: " << conn->getLastError() << std::endl;
            return false;
        }
        
        std::cout << "✓ Connection successful!" << std::endl;
        
        // Count tables
        std::cout << "\nCounting tables in database..." << std::endl;
        auto stmt = conn->createStatement("SELECT COUNT(*) as table_count FROM information_schema.tables WHERE table_schema='public'");
        auto result = stmt->executeQuery();
        
        if (result && result->next()) {
            int tableCount = result->getInt(0);
            std::cout << "✓ Total tables: " << tableCount << std::endl;
        }
        
        conn->close();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
#else
    std::cerr << "Error: PostgreSQL support not compiled in. Rebuild with -DWITH_POSTGRESQL=ON" << std::endl;
    return false;
#endif
}

// Test catalog functionality
bool testCatalog(std::shared_ptr<IConnection> conn, const std::string& dbType, bool showDetails) {
    printSeparator("Testing Catalog Functionality");
    
    try {
        std::cout << "Fetching table list from " << dbType << "..." << std::endl;
        
        std::string query;
        if (dbType == "postgresql") {
#ifdef WITH_POSTGRESQL
            query = "SELECT table_name FROM information_schema.tables WHERE table_schema='public' ORDER BY table_name";
#else
            std::cerr << "Error: PostgreSQL support not compiled in." << std::endl;
            return false;
#endif
        } else if (dbType == "sybase") {
#ifdef WITH_SYBASE
            query = "SELECT name FROM sysobjects WHERE type='U' ORDER BY name";
#else
            std::cerr << "Error: Sybase support not compiled in." << std::endl;
            return false;
#endif
        }
        
        auto stmt = conn->createStatement(query);
        auto result = stmt->executeQuery();
        
        int tableCount = 0;
        std::cout << "\n--- Tables ---" << std::endl;
        while (result && result->next()) {
            std::string tableName = result->getString(0);
            
            // Validate table name to prevent SQL injection
            if (!isValidIdentifier(tableName)) {
                std::cerr << "Warning: Skipping invalid table name: " << tableName << std::endl;
                continue;
            }
            
            std::cout << "  • " << tableName << std::endl;
            tableCount++;
            
            // Show column details if requested
            if (showDetails) {
                std::cout << "    Columns:" << std::endl;
                
                // Use parameterized query for PostgreSQL, validated identifier for Sybase
                if (dbType == "postgresql") {
                    auto colStmt = conn->createStatement(
                        "SELECT column_name, data_type, character_maximum_length, is_nullable "
                        "FROM information_schema.columns "
                        "WHERE table_name=$1 AND table_schema='public' "
                        "ORDER BY ordinal_position");
                    colStmt->bindString(1, tableName);
                    auto colResult = colStmt->executeQuery();
                    
                    while (colResult && colResult->next()) {
                        std::string colName = colResult->getString(0);
                        std::string colType = colResult->getString(1);
                        std::string colLength = colResult->isNull(2) ? "" : colResult->getString(2);
                        std::string nullable = colResult->getString(3);
                        
                        std::cout << "      - " << colName << " (" << colType;
                        if (!colLength.empty()) {
                            std::cout << "(" << colLength << ")";
                        }
                        std::cout << ", nullable: " << nullable;
                        std::cout << ")" << std::endl;
                    }
                } else if (dbType == "sybase") {
                    // Sybase doesn't support parameterized queries in the same way,
                    // but we've validated the identifier above
                    std::string colQuery = 
                        "SELECT c.name, t.name, c.length, c.status "
                        "FROM syscolumns c, systypes t "
                        "WHERE c.id = OBJECT_ID('" + tableName + "') "
                        "AND c.usertype = t.usertype "
                        "ORDER BY c.colid";
                    
                    auto colStmt = conn->createStatement(colQuery);
                    auto colResult = colStmt->executeQuery();
                    
                    while (colResult && colResult->next()) {
                        std::string colName = colResult->getString(0);
                        std::string colType = colResult->getString(1);
                        std::string colLength = colResult->isNull(2) ? "" : colResult->getString(2);
                        
                        std::cout << "      - " << colName << " (" << colType;
                        if (!colLength.empty()) {
                            std::cout << "(" << colLength << ")";
                        }
                        std::cout << ")" << std::endl;
                    }
                }
            }
        }
        
        std::cout << "\n✓ Total tables found: " << tableCount << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

// Test DBReader with product table
bool testDBReader(std::shared_ptr<IConnection> conn, const std::string& dbType) {
    printSeparator("Testing DBReader");
    
    try {
        // Register Product entity
        hft::catalog::registerEntity<Product>();
        
        // Generate CREATE TABLE SQL
        std::string createSQL = Catalog::instance().generateCreateTableSQL("products", dbType);
        
        std::cout << "Creating products table..." << std::endl;
        conn->execute("DROP TABLE IF EXISTS products");
        conn->execute(createSQL);
        std::cout << "✓ Table created" << std::endl;
        
        // Create repository
        Repository<Product> productRepo(conn);
        
        // Insert test data
        std::cout << "\nInserting test products..." << std::endl;
        
        Product p1;
        p1.name = "Laptop";
        p1.description = "High-performance laptop";
        p1.price = 1299.99;
        p1.quantity = 5;
        productRepo.insert(p1);
        
        Product p2;
        p2.name = "Mouse";
        p2.description = "Wireless mouse";
        p2.price = 29.99;
        p2.quantity = 50;
        productRepo.insert(p2);
        
        Product p3;
        p3.name = "Keyboard";
        p3.description = "Mechanical keyboard";
        p3.price = 89.99;
        p3.quantity = 20;
        productRepo.insert(p3);
        
        std::cout << "✓ 3 products inserted" << std::endl;
        
        // Read all products using DBReader pattern
        std::cout << "\nReading products using SELECT *..." << std::endl;
        auto stmt = conn->createStatement("SELECT * FROM products ORDER BY id");
        auto result = stmt->executeQuery();
        
        std::cout << "\n" << std::left;
        std::cout << std::setw(5) << "ID" 
                  << std::setw(15) << "Name" 
                  << std::setw(30) << "Description" 
                  << std::setw(10) << "Price" 
                  << std::setw(10) << "Quantity" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        
        while (result && result->next()) {
            std::cout << std::setw(5) << result->getLong(0)
                      << std::setw(15) << result->getString(1)
                      << std::setw(30) << result->getString(2)
                      << std::setw(10) << std::fixed << std::setprecision(2) << result->getDouble(3)
                      << std::setw(10) << result->getInt(4) << std::endl;
        }
        
        std::cout << "\n✓ DBReader test completed" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

// Test JSON export
bool testJSONExport(std::shared_ptr<IConnection> conn, const std::string& tableName, const std::string& outputFile) {
    printSeparator("Testing JSON Export");
    
    try {
        // Validate table name to prevent SQL injection
        if (!isValidIdentifier(tableName)) {
            std::cerr << "Error: Invalid table name. Table names must contain only letters, digits, and underscores." << std::endl;
            return false;
        }
        
        std::cout << "Exporting table '" << tableName << "' to JSON..." << std::endl;
        
        // Use validated identifier in query (safe after validation)
        auto stmt = conn->createStatement("SELECT * FROM " + tableName);
        auto result = stmt->executeQuery();
        
        if (!result) {
            std::cerr << "Failed to query table" << std::endl;
            return false;
        }
        
        // Build JSON array
        json jsonArray = json::array();
        int columnCount = result->getColumnCount();
        
        while (result->next()) {
            json row = json::object();
            
            for (int i = 0; i < columnCount; i++) {
                std::string colName = result->getColumnName(i);
                
                if (result->isNull(i)) {
                    row[colName] = nullptr;
                } else {
                    // Try to get value as different types
                    // This is a simplified approach - in production, you'd want to check column types
                    try {
                        row[colName] = result->getString(i);
                    } catch (...) {
                        row[colName] = nullptr;
                    }
                }
            }
            
            jsonArray.push_back(row);
        }
        
        // Create output object with metadata
        json output;
        output["table"] = tableName;
        output["record_count"] = jsonArray.size();
        output["data"] = jsonArray;
        
        // Write to file
        std::ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open output file: " << outputFile << std::endl;
            return false;
        }
        
        outFile << output.dump(2); // Pretty print with 2-space indent
        outFile.close();
        
        std::cout << "✓ Exported " << jsonArray.size() << " records to: " << outputFile << std::endl;
        std::cout << "✓ JSON export completed" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    try {
        cxxopts::Options options("HFT-Demo", "HFT Legacy Migration Framework - Demo Application");
        
        options.add_options()
            ("s,sybase", "Use Sybase database")
            ("p,postgresql", "Use PostgreSQL database")
            ("u,user", "Database user", cxxopts::value<std::string>()->default_value(""))
            ("w,password", "Database password", cxxopts::value<std::string>()->default_value(""))
            ("host", "PostgreSQL host (default: localhost)", cxxopts::value<std::string>()->default_value("localhost"))
            ("server", "Sybase server name", cxxopts::value<std::string>()->default_value(""))
            ("dbname", "Database name", cxxopts::value<std::string>()->default_value(""))
            ("test-connection", "Test database connection")
            ("test-catalog", "Test catalog functionality")
            ("test-dbreader", "Test DBReader functionality")
            ("test-json", "Test JSON export", cxxopts::value<std::string>()->implicit_value("products"))
            ("d,details", "Show detailed information (for catalog)")
            ("o,output", "Output file path", cxxopts::value<std::string>()->default_value("output.json"))
            ("h,help", "Print usage");
        
        auto result = options.parse(argc, argv);
        
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            std::cout << "\nSecurity Note:" << std::endl;
            std::cout << "  For production use, avoid passing credentials via command-line arguments." << std::endl;
            std::cout << "  Consider using environment variables or configuration files with restricted permissions." << std::endl;
            std::cout << "\nExamples:" << std::endl;
            std::cout << "  Test PostgreSQL connection:" << std::endl;
            std::cout << "    ./HFT-Demo -p --test-connection -u postgres -w password --dbname testdb" << std::endl;
            std::cout << "    ./HFT-Demo -p --test-connection -u postgres -w password --dbname testdb --host 192.168.1.100" << std::endl;
            std::cout << "\n  Test Sybase connection:" << std::endl;
            std::cout << "    ./HFT-Demo -s --test-connection -u sa -w password --server SYBASE_SERVER" << std::endl;
            std::cout << "\n  Test catalog with details:" << std::endl;
            std::cout << "    ./HFT-Demo -p --test-catalog -d -u postgres -w password --dbname testdb" << std::endl;
            std::cout << "\n  Test DBReader:" << std::endl;
            std::cout << "    ./HFT-Demo -p --test-dbreader -u postgres -w password --dbname testdb" << std::endl;
            std::cout << "\n  Export table to JSON:" << std::endl;
            std::cout << "    ./HFT-Demo -p --test-json products -o products.json -u postgres -w password --dbname testdb" << std::endl;
            return 0;
        }
        
        // Determine database type
        bool useSybase = result.count("sybase") > 0;
        bool usePostgreSQL = result.count("postgresql") > 0;
        
        if (!useSybase && !usePostgreSQL) {
            std::cerr << "Error: Must specify either --sybase or --postgresql" << std::endl;
            std::cout << "\nUse --help for usage information" << std::endl;
            return 1;
        }
        
        if (useSybase && usePostgreSQL) {
            std::cerr << "Error: Cannot use both --sybase and --postgresql simultaneously" << std::endl;
            return 1;
        }
        
        printSeparator("HFT-Demo - Legacy Migration Framework");
        std::cout << "Database Type: " << (useSybase ? "Sybase" : "PostgreSQL") << std::endl;
        
        // Get connection parameters
        std::string user = result["user"].as<std::string>();
        std::string password = result["password"].as<std::string>();
        std::string server = result["server"].as<std::string>();
        std::string dbname = result["dbname"].as<std::string>();
        std::string host = result["host"].as<std::string>();
        
        // Test connection
        if (result.count("test-connection")) {
            if (useSybase) {
                if (user.empty() || password.empty() || server.empty()) {
                    std::cerr << "Error: Sybase connection requires --user, --password, and --server" << std::endl;
                    return 1;
                }
                return testSybaseConnection(user, password, server) ? 0 : 1;
            } else {
                if (user.empty() || password.empty() || dbname.empty()) {
                    std::cerr << "Error: PostgreSQL connection requires --user, --password, and --dbname" << std::endl;
                    return 1;
                }
                return testPostgreSQLConnection(user, password, dbname, host) ? 0 : 1;
            }
        }
        
        // For other tests, we need to establish a connection
        std::shared_ptr<IConnection> conn;
        std::string dbType;
        
        if (useSybase) {
#ifdef WITH_SYBASE
            if (user.empty() || password.empty() || server.empty()) {
                std::cerr << "Error: Sybase connection requires --user, --password, and --server" << std::endl;
                return 1;
            }
            conn = std::make_shared<SybaseConnection>();
            std::string connStr = "user=" + user + ";password=" + password + ";server=" + server;
            if (!conn->open(connStr)) {
                std::cerr << "Failed to connect: " << conn->getLastError() << std::endl;
                return 1;
            }
            dbType = "sybase";
#else
            std::cerr << "Error: Sybase support not compiled in. Rebuild with -DWITH_SYBASE=ON" << std::endl;
            return 1;
#endif
        } else {
#ifdef WITH_POSTGRESQL
            if (user.empty() || password.empty() || dbname.empty()) {
                std::cerr << "Error: PostgreSQL connection requires --user, --password, and --dbname" << std::endl;
                return 1;
            }
            conn = std::make_shared<PostgreSQLConnection>();
            std::string connStr = "host=" + host + " dbname=" + dbname + " user=" + user + " password=" + password;
            if (!conn->open(connStr)) {
                std::cerr << "Failed to connect: " << conn->getLastError() << std::endl;
                return 1;
            }
            dbType = "postgresql";
#else
            std::cerr << "Error: PostgreSQL support not compiled in. Rebuild with -DWITH_POSTGRESQL=ON" << std::endl;
            return 1;
#endif
        }
        
        std::cout << "✓ Connected to " << dbType << std::endl;
        
        bool success = true;
        
        // Test catalog
        if (result.count("test-catalog")) {
            bool showDetails = result.count("details") > 0;
            success = testCatalog(conn, dbType, showDetails) && success;
        }
        
        // Test DBReader
        if (result.count("test-dbreader")) {
            success = testDBReader(conn, dbType) && success;
        }
        
        // Test JSON export
        if (result.count("test-json")) {
            std::string tableName = result["test-json"].as<std::string>();
            std::string outputFile = result["output"].as<std::string>();
            success = testJSONExport(conn, tableName, outputFile) && success;
        }
        
        if (conn) {
            conn->close();
            std::cout << "\n✓ Connection closed" << std::endl;
        }
        
        printSeparator("Demo Completed");
        
        return success ? 0 : 1;
        
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
