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

private:
    void loadSybase(IDBConnection& conn);
    void loadPostgres(IDBConnection& conn);

    std::unordered_map<std::string, TableMeta> _tables;
};
