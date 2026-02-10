#include "pg/PgConnection.hpp"
#include "db/DBException.hpp"
#include "pg/PgReader.hpp"
#include "pg/PgPreparedStatement.hpp"
#include "pg/PgTransaction.hpp"

#ifdef WITH_POSTGRESQL
#include <libpq-fe.h>
#endif

PgConnection::PgConnection(const std::string& conninfo)
    : _conninfo(conninfo) {
#ifdef WITH_POSTGRESQL
    _conn = PQconnectdb(conninfo.c_str());
    if (PQstatus(_conn) != CONNECTION_OK) {
        std::string error = PQerrorMessage(_conn);
        PQfinish(_conn);
        _conn = nullptr;
        throw DBException("PostgreSQL connection failed: " + error);
    }
#else
    throw DBException("PostgreSQL support not compiled in");
#endif
}

PgConnection::~PgConnection() {
#ifdef WITH_POSTGRESQL
    if (_conn) {
        PQfinish(_conn);
        _conn = nullptr;
    }
#endif
}

std::unique_ptr<IDBReader>
PgConnection::executeQuery(const std::string& sql) {
#ifdef WITH_POSTGRESQL
    if (!_conn) {
        throw DBException("PgConnection::executeQuery: Connection is null");
    }
    
    PGresult* res = PQexec(_conn, sql.c_str());
    if (!res) {
        throw DBException("PgConnection::executeQuery: PQexec failed");
    }
    
    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        std::string error = PQerrorMessage(_conn);
        PQclear(res);
        throw DBException("PgConnection::executeQuery: " + error);
    }
    
    return std::make_unique<PgReader>(res);
#else
    (void)sql;
    throw DBException("PostgreSQL support not compiled in");
#endif
}

std::unique_ptr<IDBPreparedStatement>
PgConnection::prepare(const std::string& sql) {
#ifdef WITH_POSTGRESQL
    if (!_conn) {
        throw DBException("PgConnection::prepare: Connection is null");
    }
    return std::make_unique<PgPreparedStatement>(sql, this);
#else
    (void)sql;
    throw DBException("PostgreSQL support not compiled in");
#endif
}

std::unique_ptr<IDBTransaction>
PgConnection::beginTransaction() {
#ifdef WITH_POSTGRESQL
    if (!_conn) {
        throw DBException("PgConnection::beginTransaction: Connection is null");
    }
    
    PGresult* res = PQexec(_conn, "BEGIN");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::string error = _conn ? PQerrorMessage(_conn) : "Connection is null";
        if (res) PQclear(res);
        throw DBException("PgConnection::beginTransaction: " + error);
    }
    PQclear(res);
    
    return std::make_unique<PgTransaction>(this);
#else
    throw DBException("PostgreSQL support not compiled in");
#endif
}
