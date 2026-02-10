#include "pg/PgTransaction.hpp"
#include "db/DBException.hpp"
#include "pg/PgConnection.hpp"

#ifdef WITH_POSTGRESQL
#include <libpq-fe.h>
#endif

PgTransaction::PgTransaction(PgConnection* conn)
    : _conn(conn) {}

PgTransaction::~PgTransaction() {
    // Auto-rollback if still active
    if (_active && _conn) {
        try {
            rollback();
        } catch (...) {
            // Ignore errors in destructor
        }
    }
}

void PgTransaction::commit() {
#ifdef WITH_POSTGRESQL
    if (!_active) throw DBException("PgTransaction::commit: not active");
    if (!_conn || !_conn->getConnection()) {
        throw DBException("PgTransaction::commit: Connection is null");
    }
    
    PGresult* res = PQexec(_conn->getConnection(), "COMMIT");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::string error = _conn->getConnection() ? PQerrorMessage(_conn->getConnection()) : "Connection is null";
        if (res) PQclear(res);
        throw DBException("PgTransaction::commit: " + error);
    }
    PQclear(res);
    _active = false;
#else
    throw DBException("PostgreSQL support not compiled in");
#endif
}

void PgTransaction::rollback() {
#ifdef WITH_POSTGRESQL
    if (!_active) throw DBException("PgTransaction::rollback: not active");
    if (!_conn || !_conn->getConnection()) {
        throw DBException("PgTransaction::rollback: Connection is null");
    }
    
    PGresult* res = PQexec(_conn->getConnection(), "ROLLBACK");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::string error = _conn->getConnection() ? PQerrorMessage(_conn->getConnection()) : "Connection is null";
        if (res) PQclear(res);
        throw DBException("PgTransaction::rollback: " + error);
    }
    PQclear(res);
    _active = false;
#else
    throw DBException("PostgreSQL support not compiled in");
#endif
}
