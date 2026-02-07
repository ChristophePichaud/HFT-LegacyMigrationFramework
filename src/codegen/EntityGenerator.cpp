#include "EntityGenerator.hpp"
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
        std::cerr << "[EntityGenerator] Failed to create directory: " << e.what() << "\n";
    }
}

// Helper function to map SQL types to C++ types
static std::string mapSQLTypeToCpp(const std::string& sqlType) {
    std::string lowerType = sqlType;
    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
    
    if (lowerType.find("int") != std::string::npos) {
        if (lowerType.find("bigint") != std::string::npos) return "int64_t";
        if (lowerType.find("smallint") != std::string::npos) return "int16_t";
        if (lowerType.find("tinyint") != std::string::npos) return "int8_t";
        return "int";
    }
    if (lowerType.find("float") != std::string::npos || 
        lowerType.find("double") != std::string::npos ||
        lowerType.find("real") != std::string::npos) {
        return "double";
    }
    if (lowerType.find("numeric") != std::string::npos || 
        lowerType.find("decimal") != std::string::npos) {
        return "double";
    }
    if (lowerType.find("bool") != std::string::npos || 
        lowerType.find("bit") != std::string::npos) {
        return "bool";
    }
    if (lowerType.find("char") != std::string::npos || 
        lowerType.find("text") != std::string::npos ||
        lowerType.find("varchar") != std::string::npos) {
        return "std::string";
    }
    if (lowerType.find("date") != std::string::npos || 
        lowerType.find("time") != std::string::npos) {
        return "std::string";  // Simplified - could use chrono types
    }
    
    return "std::string";  // Default
}

void EntityGenerator::generateEntities(const Catalog& catalog, const std::string& outputDir) {
    ensureDirectory(outputDir);
    
    for (const auto& [tableName, tableMeta] : catalog.tables()) {
        std::ostringstream oss;
        
        // Header
        oss << "#pragma once\n";
        oss << "#include \"entity/BaseEntity.hpp\"\n";
        oss << "#include \"entity/EntityTraits.hpp\"\n";
        oss << "#include \"entity/Column.hpp\"\n";
        oss << "#include <string>\n";
        oss << "#include <cstdint>\n";
        oss << "#include <tuple>\n";
        oss << "#include <nlohmann/json.hpp>\n\n";
        
        // Class definition
        oss << "class " << tableName << " : public BaseEntity {\n";
        oss << "public:\n";
        
        // Member variables
        for (const auto& col : tableMeta.columns) {
            std::string cppType = mapSQLTypeToCpp(col.typeName);
            oss << "    " << cppType << " " << col.name;
            
            // Initialize with default value
            if (cppType == "int" || cppType == "int64_t" || 
                cppType == "int16_t" || cppType == "int8_t") {
                oss << "{0}";
            } else if (cppType == "double") {
                oss << "{0.0}";
            } else if (cppType == "bool") {
                oss << "{false}";
            } else if (cppType == "std::string") {
                oss << "{\"\"}";
            }
            oss << ";\n";
        }
        
        // toJson method
        oss << "\n    nlohmann::json toJson() const override {\n";
        oss << "        nlohmann::json j;\n";
        for (const auto& col : tableMeta.columns) {
            oss << "        j[\"" << col.name << "\"] = " << col.name << ";\n";
        }
        oss << "        return j;\n";
        oss << "    }\n";
        oss << "};\n\n";
        
        // EntityTraits specialization
        oss << "template<>\n";
        oss << "struct EntityTraits<" << tableName << "> {\n";
        oss << "    using Entity = " << tableName << ";\n\n";
        oss << "    static constexpr std::string_view tableName  = \"" << tableName << "\";\n";
        
        // Assume first column is primary key
        if (!tableMeta.columns.empty()) {
            oss << "    static constexpr std::string_view primaryKey = \"" << tableMeta.columns[0].name << "\";\n\n";
        }
        
        // Columns tuple
        oss << "    static constexpr auto columns = std::make_tuple(\n";
        bool first = true;
        for (const auto& col : tableMeta.columns) {
            if (!first) oss << ",\n";
            std::string cppType = mapSQLTypeToCpp(col.typeName);
            oss << "        Column<Entity, " << cppType << ">{ \"" << col.name << "\", &Entity::" << col.name << " }";
            first = false;
        }
        oss << "\n    );\n";
        oss << "};\n";
        
        // Write to file
        std::string filename = outputDir + "/" + tableName + ".hpp";
        std::ofstream outFile(filename);
        if (outFile) {
            outFile << oss.str();
            std::cout << "[EntityGenerator] Generated: " << filename << "\n";
        } else {
            std::cerr << "[EntityGenerator] Failed to write: " << filename << "\n";
        }
    }
}

void EntityGenerator::generateEntityTraits(const Catalog& catalog, const std::string& outputDir) {
    // Entity traits are generated inline with entities in generateEntities()
    // This method can be used for separate traits file if needed
    std::cout << "[EntityGenerator] EntityTraits generated inline with entities\n";
    (void)catalog;
    (void)outputDir;
}
