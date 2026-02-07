#include "Catalog.hpp"
#include "db/IDBConnection.hpp"
#include "db/IDBReader.hpp"
#include "db/IDBRow.hpp"
#include "db/IDBValue.hpp"
#include "db/DBException.hpp"
#include <sstream>

Catalog::Catalog(IDBConnection& conn, DbDialect dialect) {
    switch (dialect) {
    case DbDialect::Sybase:
        loadSybase(conn);
        break;
    case DbDialect::PostgreSQL:
        loadPostgres(conn);
        break;
    }
}

const TableMeta* Catalog::findTable(const std::string& name) const {
    auto it = _tables.find(name);
    if (it == _tables.end()) return nullptr;
    return &it->second;
}

const std::unordered_map<std::string, TableMeta>& Catalog::tables() const {
    return _tables;
}

std::string Catalog::generateCreateTableSQL(const std::string& tableName, DbDialect dialect) const {
    const TableMeta* table = findTable(tableName);
    if (!table) {
        throw DBException("Table not found: " + tableName);
    }
    
    std::ostringstream oss;
    oss << "CREATE TABLE " << tableName << " (\n";
    
    bool first = true;
    for (const auto& col : table->columns) {
        if (!first) oss << ",\n";
        
        oss << "  " << col.name << " ";
        oss << mapTypeToSQL(col.typeName, col.length, col.scale, dialect);
        
        if (!col.nullable) {
            oss << " NOT NULL";
        }
        
        first = false;
    }
    
    oss << "\n)";
    
    // Add dialect-specific options
    if (dialect == DbDialect::Sybase) {
        oss << " LOCK DATAROWS";
    }
    
    return oss.str();
}

std::string Catalog::generateDropTableSQL(const std::string& tableName, DbDialect dialect) const {
    std::ostringstream oss;
    
    if (dialect == DbDialect::PostgreSQL) {
        oss << "DROP TABLE IF EXISTS " << tableName << " CASCADE";
    } else {
        oss << "DROP TABLE " << tableName;
    }
    
    return oss.str();
}

std::string Catalog::mapTypeToSQL(const std::string& typeName, int length, int scale, DbDialect dialect) const {
    // Map database types to standard SQL types per dialect
    std::string lowerType = typeName;
    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
    
    if (dialect == DbDialect::PostgreSQL) {
        // PostgreSQL type mappings
        if (lowerType.find("int") != std::string::npos) return "INTEGER";
        if (lowerType.find("bigint") != std::string::npos) return "BIGINT";
        if (lowerType.find("smallint") != std::string::npos) return "SMALLINT";
        if (lowerType.find("varchar") != std::string::npos || lowerType.find("character varying") != std::string::npos) {
            if (length > 0) return "VARCHAR(" + std::to_string(length) + ")";
            return "VARCHAR(255)";
        }
        if (lowerType.find("char") != std::string::npos && lowerType.find("varying") == std::string::npos) {
            if (length > 0) return "CHAR(" + std::to_string(length) + ")";
            return "CHAR(1)";
        }
        if (lowerType.find("text") != std::string::npos) return "TEXT";
        if (lowerType.find("numeric") != std::string::npos || lowerType.find("decimal") != std::string::npos) {
            if (length > 0 && scale > 0) {
                return "NUMERIC(" + std::to_string(length) + "," + std::to_string(scale) + ")";
            }
            return "NUMERIC";
        }
        if (lowerType.find("real") != std::string::npos) return "REAL";
        if (lowerType.find("double") != std::string::npos || lowerType.find("float") != std::string::npos) return "DOUBLE PRECISION";
        if (lowerType.find("bool") != std::string::npos) return "BOOLEAN";
        if (lowerType.find("date") != std::string::npos) return "DATE";
        if (lowerType.find("timestamp") != std::string::npos) return "TIMESTAMP";
        if (lowerType.find("time") != std::string::npos) return "TIME";
        
    } else if (dialect == DbDialect::Sybase) {
        // Sybase type mappings
        if (lowerType.find("int") != std::string::npos) return "INT";
        if (lowerType.find("bigint") != std::string::npos) return "BIGINT";
        if (lowerType.find("smallint") != std::string::npos) return "SMALLINT";
        if (lowerType.find("tinyint") != std::string::npos) return "TINYINT";
        if (lowerType.find("varchar") != std::string::npos) {
            if (length > 0) return "VARCHAR(" + std::to_string(length) + ")";
            return "VARCHAR(255)";
        }
        if (lowerType.find("char") != std::string::npos && lowerType.find("var") == std::string::npos) {
            if (length > 0) return "CHAR(" + std::to_string(length) + ")";
            return "CHAR(1)";
        }
        if (lowerType.find("text") != std::string::npos) return "TEXT";
        if (lowerType.find("numeric") != std::string::npos || lowerType.find("decimal") != std::string::npos) {
            if (length > 0 && scale > 0) {
                return "NUMERIC(" + std::to_string(length) + "," + std::to_string(scale) + ")";
            }
            return "NUMERIC(18,2)";
        }
        if (lowerType.find("real") != std::string::npos) return "REAL";
        if (lowerType.find("float") != std::string::npos || lowerType.find("double") != std::string::npos) return "FLOAT";
        if (lowerType.find("bit") != std::string::npos) return "BIT";
        if (lowerType.find("date") != std::string::npos) return "DATE";
        if (lowerType.find("datetime") != std::string::npos) return "DATETIME";
        if (lowerType.find("time") != std::string::npos) return "TIME";
    }
    
    // Default: return the original type name
    return typeName;
}

void Catalog::loadSybase(IDBConnection& conn) {
    // Query Sybase system tables to get schema information
    // Query for all user tables
    std::string tableQuery = 
        "SELECT name FROM sysobjects WHERE type = 'U' ORDER BY name";
    
    auto tableReader = conn.executeQuery(tableQuery);
    while (tableReader->next()) {
        std::string tableName = tableReader->row()[0].asString();
        TableMeta tableMeta;
        tableMeta.name = tableName;
        
        // Query columns for this table using OBJECT_ID (safer than direct concatenation)
        // Note: In production, consider using stored procedures or more robust escaping
        std::string escapedTableName = tableName;
        // Simple escaping for Sybase - replace single quotes
        size_t pos = 0;
        while ((pos = escapedTableName.find("'", pos)) != std::string::npos) {
            escapedTableName.replace(pos, 1, "''");
            pos += 2;
        }
        
        std::string columnQuery = 
            "SELECT c.name, t.name as type_name, c.length, c.scale, c.status "
            "FROM syscolumns c "
            "JOIN systypes t ON c.usertype = t.usertype "
            "WHERE c.id = OBJECT_ID('" + escapedTableName + "') "
            "ORDER BY c.colid";
        
        auto columnReader = conn.executeQuery(columnQuery);
        while (columnReader->next()) {
            ColumnMeta colMeta;
            colMeta.name = columnReader->row()[0].asString();
            colMeta.typeName = columnReader->row()[1].asString();
            colMeta.length = columnReader->row()[2].asInt();
            colMeta.scale = columnReader->row()[3].asInt();
            
            // In Sybase, status bit 8 indicates NOT NULL
            int status = columnReader->row()[4].asInt();
            colMeta.nullable = (status & 8) == 0;
            
            tableMeta.columns.push_back(colMeta);
        }
        
        _tables[tableName] = tableMeta;
    }
}

void Catalog::loadPostgres(IDBConnection& conn) {
    // Query PostgreSQL system catalogs to get schema information
    // Query for all user tables in the public schema
    std::string tableQuery = 
        "SELECT tablename FROM pg_tables "
        "WHERE schemaname = 'public' "
        "ORDER BY tablename";
    
    auto tableReader = conn.executeQuery(tableQuery);
    while (tableReader->next()) {
        std::string tableName = tableReader->row()[0].asString();
        TableMeta tableMeta;
        tableMeta.name = tableName;
        
        // Query columns for this table with proper escaping
        // Note: Table names from pg_tables are already validated by PostgreSQL
        std::string escapedTableName = tableName;
        // Simple escaping - replace single quotes
        size_t pos = 0;
        while ((pos = escapedTableName.find("'", pos)) != std::string::npos) {
            escapedTableName.replace(pos, 1, "''");
            pos += 2;
        }
        
        std::string columnQuery = 
            "SELECT "
            "  a.attname AS column_name, "
            "  pg_catalog.format_type(a.atttypid, a.atttypmod) AS data_type, "
            "  a.attlen AS length, "
            "  a.atttypmod AS type_modifier, "
            "  NOT a.attnotnull AS is_nullable "
            "FROM pg_catalog.pg_attribute a "
            "JOIN pg_catalog.pg_class c ON a.attrelid = c.oid "
            "JOIN pg_catalog.pg_namespace n ON c.relnamespace = n.oid "
            "WHERE c.relname = '" + escapedTableName + "' "
            "  AND n.nspname = 'public' "
            "  AND a.attnum > 0 "
            "  AND NOT a.attisdropped "
            "ORDER BY a.attnum";
        
        auto columnReader = conn.executeQuery(columnQuery);
        while (columnReader->next()) {
            ColumnMeta colMeta;
            colMeta.name = columnReader->row()[0].asString();
            colMeta.typeName = columnReader->row()[1].asString();
            colMeta.length = columnReader->row()[2].asInt();
            
            // Extract scale from type_modifier if available
            int typeMod = columnReader->row()[3].asInt();
            if (typeMod >= 0) {
                colMeta.scale = (typeMod - 4) & 0xFFFF;
            } else {
                colMeta.scale = 0;
            }
            
            colMeta.nullable = columnReader->row()[4].asInt() != 0;
            
            tableMeta.columns.push_back(colMeta);
        }
        
        _tables[tableName] = tableMeta;
    }
}
