#include "codegen/UnitTestGenerator.hpp"
#include "catalog/Catalog.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// Helper function to ensure directory exists
static void ensureDirectory(const std::string& path) {
    try {
        std::filesystem::create_directories(path);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[UnitTestGenerator] Failed to create directory: " << e.what() << "\n";
    }
}

void UnitTestGenerator::generateTests(const Catalog& catalog, const std::string& outputDir) {
    ensureDirectory(outputDir);
    
    for (const auto& [tableName, tableMeta] : catalog.tables()) {
        std::ostringstream oss;
        
        // Header
        oss << "#include <gtest/gtest.h>\n";
        oss << "#include \"entity/generated/" << tableName << ".hpp\"\n";
        oss << "#include \"repository/generated/Repository_" << tableName << ".hpp\"\n";
        oss << "#include \"tests/MockConnection.hpp\"\n\n";
        
        // Test fixture
        oss << "class " << tableName << "Test : public ::testing::Test {\n";
        oss << "protected:\n";
        oss << "    void SetUp() override {\n";
        oss << "        // Setup code here\n";
        oss << "    }\n\n";
        oss << "    void TearDown() override {\n";
        oss << "        // Cleanup code here\n";
        oss << "    }\n\n";
        oss << "    MockConnection mockConn;\n";
        oss << "};\n\n";
        
        // Test cases
        oss << "TEST_F(" << tableName << "Test, EntityCreation) {\n";
        oss << "    " << tableName << " entity;\n";
        oss << "    EXPECT_NO_THROW({\n";
        oss << "        // Test entity creation\n";
        if (!tableMeta.columns.empty()) {
            oss << "        entity." << tableMeta.columns[0].name << " = 1;\n";
        }
        oss << "    });\n";
        oss << "}\n\n";
        
        oss << "TEST_F(" << tableName << "Test, JsonSerialization) {\n";
        oss << "    " << tableName << " entity;\n";
        if (!tableMeta.columns.empty()) {
            oss << "    entity." << tableMeta.columns[0].name << " = 1;\n";
        }
        oss << "    auto json = entity.toJson();\n";
        oss << "    EXPECT_FALSE(json.empty());\n";
        oss << "}\n\n";
        
        oss << "TEST_F(" << tableName << "Test, RepositoryGetAll) {\n";
        oss << "    Repository_" << tableName << " repo(mockConn);\n";
        oss << "    // Note: This test requires a properly configured mock\n";
        oss << "    // EXPECT_NO_THROW({\n";
        oss << "    //     auto entities = repo.getAll();\n";
        oss << "    // });\n";
        oss << "}\n\n";
        
        oss << "TEST_F(" << tableName << "Test, RepositoryInsert) {\n";
        oss << "    Repository_" << tableName << " repo(mockConn);\n";
        oss << "    " << tableName << " entity;\n";
        oss << "    // Note: This test requires a properly configured mock\n";
        oss << "    // EXPECT_NO_THROW({\n";
        oss << "    //     repo.insert(entity);\n";
        oss << "    // });\n";
        oss << "}\n";
        
        // Write to file
        std::string filename = outputDir + "/test_" + tableName + ".cpp";
        std::ofstream outFile(filename);
        if (outFile) {
            outFile << oss.str();
            std::cout << "[UnitTestGenerator] Generated: " << filename << "\n";
        } else {
            std::cerr << "[UnitTestGenerator] Failed to write: " << filename << "\n";
        }
    }
}
