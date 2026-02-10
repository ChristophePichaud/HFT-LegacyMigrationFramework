#pragma once
#include <unordered_map>
#include <string>
#include "TableMeta.hpp"
#include "DbDialect.hpp"

class IDBConnection;

class Catalog {
public:
    Catalog(IDBConnection& conn, DbDialect dialect);

    const TableMeta* findTable(const std::string& name) const;
    const std::unordered_map<std::string, TableMeta>& tables() const;
    
    // SQL DDL generation methods
    std::string generateCreateTableSQL(const std::string& tableName, DbDialect dialect) const;
    std::string generateDropTableSQL(const std::string& tableName, DbDialect dialect) const;

private:
    void loadSybase(IDBConnection& conn);
    void loadPostgres(IDBConnection& conn);
    
    std::string mapTypeToSQL(const std::string& typeName, int length, int scale, DbDialect dialect) const;

    std::unordered_map<std::string, TableMeta> _tables;
};
