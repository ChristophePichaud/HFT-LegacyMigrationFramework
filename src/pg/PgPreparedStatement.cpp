#include "pg/PgPreparedStatement.hpp"
#include "db/DBException.hpp"
#include "pg/PgReader.hpp"
#include "pg/PgConnection.hpp"

#ifdef WITH_POSTGRESQL
#include <libpq-fe.h>
#endif

PgPreparedStatement::PgPreparedStatement(std::string sql, PgConnection* conn)
    : _sql(std::move(sql)), _conn(conn) {}

PgPreparedStatement::~PgPreparedStatement() = default;

void PgPreparedStatement::bindInt(int index, int value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindInt: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void PgPreparedStatement::bindDouble(int index, double value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindDouble: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void PgPreparedStatement::bindString(int index, const std::string& value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindString: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = value;
}

std::unique_ptr<IDBReader> PgPreparedStatement::executeQuery() {
#ifdef WITH_POSTGRESQL
    if (!_conn || !_conn->getConnection()) {
        throw DBException("PgPreparedStatement::executeQuery: Connection is null");
    }
    
    // Convert params to C-style arrays for libpq
    std::vector<const char*> paramValues;
    paramValues.reserve(_params.size());
    for (const auto& param : _params) {
        paramValues.push_back(param.c_str());
    }
    
    PGresult* res = PQexecParams(
        _conn->getConnection(),
        _sql.c_str(),
        static_cast<int>(_params.size()),
        nullptr,  // let PostgreSQL infer types
        paramValues.data(),
        nullptr,  // text format
        nullptr,  // text format
        0         // text format results
    );
    
    if (!res) {
        throw DBException("PgPreparedStatement::executeQuery: PQexecParams failed");
    }
    
    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK) {
        std::string error = PQerrorMessage(_conn->getConnection());
        PQclear(res);
        throw DBException("PgPreparedStatement::executeQuery: " + error);
    }
    
    return std::make_unique<PgReader>(res);
#else
    throw DBException("PostgreSQL support not compiled in");
#endif
}

void PgPreparedStatement::executeUpdate() {
#ifdef WITH_POSTGRESQL
    if (!_conn || !_conn->getConnection()) {
        throw DBException("PgPreparedStatement::executeUpdate: Connection is null");
    }
    
    // Convert params to C-style arrays for libpq
    std::vector<const char*> paramValues;
    paramValues.reserve(_params.size());
    for (const auto& param : _params) {
        paramValues.push_back(param.c_str());
    }
    
    PGresult* res = PQexecParams(
        _conn->getConnection(),
        _sql.c_str(),
        static_cast<int>(_params.size()),
        nullptr,  // let PostgreSQL infer types
        paramValues.data(),
        nullptr,  // text format
        nullptr,  // text format
        0         // text format results
    );
    
    if (!res) {
        throw DBException("PgPreparedStatement::executeUpdate: PQexecParams failed");
    }
    
    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK) {
        std::string error = PQerrorMessage(_conn->getConnection());
        PQclear(res);
        throw DBException("PgPreparedStatement::executeUpdate: " + error);
    }
    
    PQclear(res);
#else
    throw DBException("PostgreSQL support not compiled in");
#endif
}
