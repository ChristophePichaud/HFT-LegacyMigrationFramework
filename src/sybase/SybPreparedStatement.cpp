#include "SybPreparedStatement.hpp"
#include "db/DBException.hpp"
#include "sybase/SybReader.hpp"
#include "sybase/SybConnection.hpp"
#include <sstream>

#ifdef WITH_SYBASE
#include <sybfront.h>
#include <sybdb.h>
#endif

SybPreparedStatement::SybPreparedStatement(std::string sql, SybConnection* conn)
    : _sql(std::move(sql)), _conn(conn) {}

SybPreparedStatement::~SybPreparedStatement() = default;

void SybPreparedStatement::bindInt(int index, int value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindInt: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void SybPreparedStatement::bindDouble(int index, double value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindDouble: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void SybPreparedStatement::bindString(int index, const std::string& value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindString: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    // Escape single quotes for Sybase
    std::string escaped;
    for (char c : value) {
        if (c == '\'') {
            escaped += "''";  // Double the quote
        } else {
            escaped += c;
        }
    }
    _params[index - 1] = "'" + escaped + "'";
}

std::string SybPreparedStatement::buildFinalSQL() {
    // Replace $1, $2, etc. with actual parameter values
    std::string result = _sql;
    for (size_t i = 0; i < _params.size(); ++i) {
        std::string placeholder = "$" + std::to_string(i + 1);
        
        // Replace all occurrences of this placeholder
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), _params[i]);
            pos += _params[i].length();
        }
    }
    return result;
}

std::unique_ptr<IDBReader> SybPreparedStatement::executeQuery() {
#ifdef WITH_SYBASE
    if (!_conn || !_conn->getDbProcess()) {
        throw DBException("SybPreparedStatement::executeQuery: Connection is null");
    }
    
    DBPROCESS* dbproc = _conn->getDbProcess();
    std::string finalSQL = buildFinalSQL();
    
    // Send SQL command
    if (dbcmd(dbproc, finalSQL.c_str()) == FAIL) {
        throw DBException("SybPreparedStatement::executeQuery: dbcmd failed");
    }
    
    // Execute query
    if (dbsqlexec(dbproc) == FAIL) {
        throw DBException("SybPreparedStatement::executeQuery: dbsqlexec failed");
    }
    
    // Get first result set
    if (dbresults(dbproc) != SUCCEED) {
        throw DBException("SybPreparedStatement::executeQuery: dbresults failed");
    }
    
    return std::make_unique<SybReader>(dbproc);
#else
    throw DBException("Sybase support not compiled in");
#endif
}

void SybPreparedStatement::executeUpdate() {
#ifdef WITH_SYBASE
    if (!_conn || !_conn->getDbProcess()) {
        throw DBException("SybPreparedStatement::executeUpdate: Connection is null");
    }
    
    DBPROCESS* dbproc = _conn->getDbProcess();
    std::string finalSQL = buildFinalSQL();
    
    // Send SQL command
    if (dbcmd(dbproc, finalSQL.c_str()) == FAIL) {
        throw DBException("SybPreparedStatement::executeUpdate: dbcmd failed");
    }
    
    // Execute update
    if (dbsqlexec(dbproc) == FAIL) {
        throw DBException("SybPreparedStatement::executeUpdate: dbsqlexec failed");
    }
    
    // Consume results
    while (dbresults(dbproc) != NO_MORE_RESULTS) {
        // Empty loop to consume all results
    }
#else
    throw DBException("Sybase support not compiled in");
#endif
}
