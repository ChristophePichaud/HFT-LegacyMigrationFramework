#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "hft/reflection/EntityTraits.h"

namespace hft {
namespace catalog {

/**
 * @brief Column metadata
 */
struct ColumnInfo {
    std::string name;
    reflection::FieldType type;
    bool primaryKey;
    bool nullable;
    
    ColumnInfo(const std::string& n, reflection::FieldType t, bool pk = false, bool null = false)
        : name(n), type(t), primaryKey(pk), nullable(null) {}
};

/**
 * @brief Table metadata
 */
struct TableInfo {
    std::string name;
    std::vector<ColumnInfo> columns;
    
    TableInfo(const std::string& n) : name(n) {}
    
    void addColumn(const ColumnInfo& column) {
        columns.push_back(column);
    }
    
    const ColumnInfo* getColumn(const std::string& name) const {
        for (const auto& col : columns) {
            if (col.name == name) return &col;
        }
        return nullptr;
    }
    
    std::vector<std::string> getPrimaryKeyColumns() const {
        std::vector<std::string> pks;
        for (const auto& col : columns) {
            if (col.primaryKey) pks.push_back(col.name);
        }
        return pks;
    }
};

/**
 * @brief Schema catalog for runtime metadata management
 */
class Catalog {
public:
    static Catalog& instance() {
        static Catalog catalog;
        return catalog;
    }
    
    /**
     * @brief Register a table in the catalog
     */
    void registerTable(const TableInfo& table) {
        tables_[table.name] = table;
    }
    
    /**
     * @brief Get table information
     */
    const TableInfo* getTable(const std::string& name) const {
        auto it = tables_.find(name);
        return (it != tables_.end()) ? &it->second : nullptr;
    }
    
    /**
     * @brief Get all registered tables
     */
    std::vector<std::string> getTableNames() const {
        std::vector<std::string> names;
        for (const auto& pair : tables_) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    /**
     * @brief Generate CREATE TABLE SQL for a table
     */
    std::string generateCreateTableSQL(const std::string& tableName, const std::string& dialect = "postgresql") const {
        const TableInfo* table = getTable(tableName);
        if (!table) return "";
        
        std::string sql = "CREATE TABLE " + tableName + " (\n";
        
        for (size_t i = 0; i < table->columns.size(); ++i) {
            const auto& col = table->columns[i];
            sql += "    " + col.name + " " + getTypeSQL(col.type, dialect);
            
            if (col.primaryKey) {
                sql += " PRIMARY KEY";
            }
            
            if (!col.nullable && !col.primaryKey) {
                sql += " NOT NULL";
            }
            
            if (i < table->columns.size() - 1) {
                sql += ",";
            }
            sql += "\n";
        }
        
        sql += ")";
        return sql;
    }
    
    /**
     * @brief Clear all registered tables
     */
    void clear() {
        tables_.clear();
    }

private:
    Catalog() = default;
    
    std::string getTypeSQL(reflection::FieldType type, const std::string& dialect) const {
        if (dialect == "postgresql") {
            switch (type) {
                case reflection::FieldType::INT32: return "INTEGER";
                case reflection::FieldType::INT64: return "BIGINT";
                case reflection::FieldType::DOUBLE: return "DOUBLE PRECISION";
                case reflection::FieldType::STRING: return "TEXT";
                case reflection::FieldType::BOOL: return "BOOLEAN";
                default: return "TEXT";
            }
        } else if (dialect == "sybase") {
            switch (type) {
                case reflection::FieldType::INT32: return "INT";
                case reflection::FieldType::INT64: return "BIGINT";
                case reflection::FieldType::DOUBLE: return "FLOAT";
                case reflection::FieldType::STRING: return "VARCHAR(255)";
                case reflection::FieldType::BOOL: return "BIT";
                default: return "VARCHAR(255)";
            }
        }
        return "TEXT";
    }
    
    std::map<std::string, TableInfo> tables_;
};

/**
 * @brief Helper to register an entity type in the catalog
 */
template<typename T>
void registerEntity() {
    TableInfo table(reflection::EntityTraits<T>::tableName());
    
    T dummy{};
    reflection::EntityTraits<T>::forEachField(dummy, [&](const std::string& name, 
                                                          reflection::FieldType type,
                                                          auto* ptr,
                                                          bool isPK,
                                                          bool isNullable) {
        table.addColumn(ColumnInfo(name, type, isPK, isNullable));
    });
    
    Catalog::instance().registerTable(table);
}

} // namespace catalog
} // namespace hft
