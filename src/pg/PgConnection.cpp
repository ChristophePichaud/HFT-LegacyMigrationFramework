#include "PgConnection.hpp"
#include "db/DBException.hpp"
#include "pg/PgReader.hpp"
#include "pg/PgPreparedStatement.hpp"
#include "pg/PgTransaction.hpp"

PgConnection::PgConnection(const std::string& conninfo)
    : _conninfo(conninfo) {}

PgConnection::~PgConnection() = default;

std::unique_ptr<IDBReader>
PgConnection::executeQuery(const std::string& sql) {
    // Stub: in real life, call PQexec and wrap result
    (void)sql;
    throw DBException("PgConnection::executeQuery not implemented");
}

std::unique_ptr<IDBPreparedStatement>
PgConnection::prepare(const std::string& sql) {
    (void)sql;
    throw DBException("PgConnection::prepare not implemented");
}

std::unique_ptr<IDBTransaction>
PgConnection::beginTransaction() {
    throw DBException("PgConnection::beginTransaction not implemented");
}
