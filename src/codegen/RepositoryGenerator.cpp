#include "RepositoryGenerator.hpp"
#include "catalog/Catalog.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

// Helper function to ensure directory exists
static void ensureDirectory(const std::string& path) {
    mkdir(path.c_str(), 0755);
}

void RepositoryGenerator::generateRepositories(const Catalog& catalog, const std::string& outputDir) {
    ensureDirectory(outputDir);
    
    for (const auto& [tableName, tableMeta] : catalog.tables()) {
        std::ostringstream oss;
        
        // Header
        oss << "#pragma once\n";
        oss << "#include \"repository/Repository.hpp\"\n";
        oss << "#include \"entity/generated/" << tableName << ".hpp\"\n\n";
        
        // Repository alias/typedef
        oss << "// Repository for " << tableName << " entity\n";
        oss << "using Repository_" << tableName << " = Repository<" << tableName << ">;\n\n";
        
        // Optional: Can add custom repository methods here
        oss << "// Custom repository class with additional methods\n";
        oss << "class " << tableName << "Repository : public Repository<" << tableName << "> {\n";
        oss << "public:\n";
        oss << "    using Repository<" << tableName << ">::Repository;\n\n";
        
        oss << "    // Add custom query methods here\n";
        oss << "    // Example:\n";
        oss << "    // std::vector<" << tableName << "> findByCustomCriteria(...) { ... }\n";
        oss << "};\n";
        
        // Write to file
        std::string filename = outputDir + "/Repository_" + tableName + ".hpp";
        std::ofstream outFile(filename);
        if (outFile) {
            outFile << oss.str();
            std::cout << "[RepositoryGenerator] Generated: " << filename << "\n";
        } else {
            std::cerr << "[RepositoryGenerator] Failed to write: " << filename << "\n";
        }
    }
}
