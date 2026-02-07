#include "Catalog.hpp"
#include "db/IDBConnection.hpp"
#include "db/IDBReader.hpp"
#include "db/IDBRow.hpp"
#include "db/IDBValue.hpp"

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

void Catalog::loadSybase(IDBConnection& conn) {
    (void)conn;
    // Stub: you can implement real queries later
}

void Catalog::loadPostgres(IDBConnection& conn) {
    (void)conn;
    // Stub: you can implement real queries later
}
