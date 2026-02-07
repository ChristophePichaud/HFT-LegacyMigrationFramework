#pragma once

#include "hft/catalog/Catalog.h"
#include <string>
#include <sstream>
#include <vector>

namespace hft {
namespace codegen {

/**
 * @brief Code generator for entity classes and repositories
 */
class CodeGenerator {
public:
    /**
     * @brief Generate entity class from table metadata
     */
    static std::string generateEntity(const catalog::TableInfo& table, const std::string& ns = "models") {
        std::ostringstream out;
        
        out << "#pragma once\n\n";
        out << "#include \"hft/reflection/EntityTraits.h\"\n";
        out << "#include <string>\n";
        out << "#include <cstdint>\n\n";
        
        if (!ns.empty()) {
            out << "namespace " << ns << " {\n\n";
        }
        
        std::string className = toPascalCase(table.name);
        
        out << "/**\n";
        out << " * @brief Entity class for table " << table.name << "\n";
        out << " */\n";
        out << "struct " << className << " {\n";
        
        // Generate fields
        for (const auto& col : table.columns) {
            out << "    " << getCppType(col.type) << " " << col.name;
            
            if (col.type == reflection::FieldType::STRING) {
                out << " = \"\"";
            } else {
                out << " = 0";
            }
            
            out << ";\n";
        }
        
        out << "};\n\n";
        
        // Generate EntityTraits specialization
        out << "} // namespace " << ns << "\n\n";
        
        out << "// EntityTraits specialization\n";
        out << "namespace hft { namespace reflection {\n\n";
        out << "template<>\n";
        out << "struct EntityTraits<" << ns << "::" << className << "> {\n";
        out << "    static constexpr const char* tableName() { return \"" << table.name << "\"; }\n";
        out << "    static constexpr size_t fieldCount() { return " << table.columns.size() << "; }\n\n";
        out << "    template<typename Func>\n";
        out << "    static void forEachField(" << ns << "::" << className << "& entity, Func&& func) {\n";
        
        for (const auto& col : table.columns) {
            out << "        func(\"" << col.name << "\", ";
            out << "getFieldType<decltype(entity." << col.name << ")>(), ";
            out << "&entity." << col.name << ", ";
            out << (col.primaryKey ? "true" : "false") << ", ";
            out << (col.nullable ? "true" : "false") << ");\n";
        }
        
        out << "    }\n";
        out << "};\n\n";
        out << "}} // namespace hft::reflection\n";
        
        return out.str();
    }
    
    /**
     * @brief Generate repository class for entity
     */
    static std::string generateRepository(const catalog::TableInfo& table, const std::string& ns = "repositories") {
        std::ostringstream out;
        
        std::string className = toPascalCase(table.name);
        std::string entityClass = "models::" + className;
        
        out << "#pragma once\n\n";
        out << "#include \"hft/orm/Repository.h\"\n";
        out << "#include \"" << className << ".h\"\n\n";
        
        if (!ns.empty()) {
            out << "namespace " << ns << " {\n\n";
        }
        
        out << "/**\n";
        out << " * @brief Repository for " << className << " entity\n";
        out << " */\n";
        out << "class " << className << "Repository : public hft::orm::Repository<" << entityClass << "> {\n";
        out << "public:\n";
        out << "    using hft::orm::Repository<" << entityClass << ">::Repository;\n";
        out << "    \n";
        out << "    // Add custom query methods here\n";
        out << "};\n\n";
        
        if (!ns.empty()) {
            out << "} // namespace " << ns << "\n";
        }
        
        return out.str();
    }
    
    /**
     * @brief Generate all entities and repositories for catalog
     */
    static void generateAll(const std::string& outputDir) {
        auto& cat = catalog::Catalog::instance();
        auto tableNames = cat.getTableNames();
        
        for (const auto& tableName : tableNames) {
            const auto* table = cat.getTable(tableName);
            if (table) {
                // Generate entity
                std::string entityCode = generateEntity(*table);
                // Write to file (implementation would write to filesystem)
                
                // Generate repository
                std::string repoCode = generateRepository(*table);
                // Write to file (implementation would write to filesystem)
            }
        }
    }

private:
    static std::string getCppType(reflection::FieldType type) {
        switch (type) {
            case reflection::FieldType::INT32: return "int32_t";
            case reflection::FieldType::INT64: return "int64_t";
            case reflection::FieldType::DOUBLE: return "double";
            case reflection::FieldType::STRING: return "std::string";
            case reflection::FieldType::BOOL: return "bool";
            default: return "std::string";
        }
    }
    
    static std::string toPascalCase(const std::string& snake_case) {
        std::string result;
        bool capitalizeNext = true;
        
        for (char c : snake_case) {
            if (c == '_') {
                capitalizeNext = true;
            } else {
                if (capitalizeNext) {
                    result += std::toupper(c);
                    capitalizeNext = false;
                } else {
                    result += c;
                }
            }
        }
        
        return result;
    }
};

} // namespace codegen
} // namespace hft
