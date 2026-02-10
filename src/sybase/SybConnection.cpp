#include "sybase/SybConnection.hpp"
#include "db/DBException.hpp"
#include "sybase/SybReader.hpp"
#include "sybase/SybPreparedStatement.hpp"
#include "sybase/SybTransaction.hpp"
#include <sstream>

#ifdef WITH_SYBASE
#include <sybfront.h>
#include <sybdb.h>
#endif

#ifdef WITH_SYBASE

bool SybConnection::_initialized = false;

void SybConnection::parseConnInfo(const std::string& conninfo,
                                  std::string& server, std::string& user,
                                  std::string& password, std::string& database) {
    // Simple parser for connection strings like:
    // "server=myserver;user=myuser;password=mypass;database=mydb"
    std::istringstream iss(conninfo);
    std::string token;
    
    while (std::getline(iss, token, ';')) {
        size_t pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            
            if (key == "server") server = value;
            else if (key == "user") user = value;
            else if (key == "password") password = value;
            else if (key == "database") database = value;
        }
    }
}

SybConnection::SybConnection(const std::string& conninfo)
    : _conninfo(conninfo) {

    // Initialize DB-Lib (only once)
    if (!_initialized) {
        if (dbinit() == FAIL) {
            throw DBException("Sybase dbinit() failed");
        }
        _initialized = true;
    }
    
    // Parse connection info
    std::string server, user, password, database;
    parseConnInfo(conninfo, server, user, password, database);
    
    // Allocate login structure
    LOGINREC* login = dblogin();
    if (!login) {
        throw DBException("Sybase dblogin() failed");
    }
    
    // Set login properties
    DBSETLUSER(login, user.c_str());
    DBSETLPWD(login, password.c_str());
    if (!database.empty()) {
        DBSETLDBNAME(login, database.c_str());
    }
    
    // Open connection
    _dbproc = dbopen(login, server.c_str());
    dbloginfree(login);
    
    if (!_dbproc) {
        throw DBException("Sybase connection failed to server: " + server);
    }
    
    // Use database if specified
    if (!database.empty()) {
        if (dbuse(_dbproc, database.c_str()) == FAIL) {
            dbclose(_dbproc);
            _dbproc = nullptr;
            throw DBException("Sybase: failed to use database: " + database);
        }
    }
}

SybConnection::~SybConnection() {

    if (_dbproc) {
        dbclose(_dbproc);
        _dbproc = nullptr;
    }

}

std::unique_ptr<IDBReader>
SybConnection::executeQuery(const std::string& sql) {

    if (!_dbproc) {
        throw DBException("SybConnection::executeQuery: Connection is null");
    }
    
    // Send SQL command
    if (dbcmd(_dbproc, sql.c_str()) == FAIL) {
        throw DBException("SybConnection::executeQuery: dbcmd failed");
    }
    
    // Execute query
    if (dbsqlexec(_dbproc) == FAIL) {
        throw DBException("SybConnection::executeQuery: dbsqlexec failed");
    }
    
    // Get first result set
    if (dbresults(_dbproc) != SUCCEED) {
        throw DBException("SybConnection::executeQuery: dbresults failed");
    }
    
    return std::make_unique<SybReader>(_dbproc);
}

std::unique_ptr<IDBPreparedStatement>
SybConnection::prepare(const std::string& sql) {

    if (!_dbproc) {
        throw DBException("SybConnection::prepare: Connection is null");
    }
    return std::make_unique<SybPreparedStatement>(sql, this);
}

std::unique_ptr<IDBTransaction>
SybConnection::beginTransaction() {

    if (!_dbproc) {
        throw DBException("SybConnection::beginTransaction: Connection is null");
    }
    
    // Send BEGIN TRAN command
    if (dbcmd(_dbproc, "BEGIN TRAN") == FAIL) {
        throw DBException("SybConnection::beginTransaction: dbcmd failed");
    }
    
    if (dbsqlexec(_dbproc) == FAIL) {
        throw DBException("SybConnection::beginTransaction: dbsqlexec failed");
    }
    
    // Consume results
    while (dbresults(_dbproc) != NO_MORE_RESULTS) {
        // Empty loop to consume all results
    }
    
    return std::make_unique<SybTransaction>(this);
// #else
//     throw DBException("Sybase support not compiled in");

}

#endif
