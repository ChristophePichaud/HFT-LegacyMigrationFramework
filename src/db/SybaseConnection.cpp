#include "hft/db/SybaseConnection.h"
#include <cstring>
#include <sstream>
#include <iostream>

#ifdef WITH_SYBASE

namespace hft {
namespace db {

bool SybaseConnection::initialized_ = false;

// Error handlers
static int err_handler(DBPROCESS* dbproc, int severity, int dberr, int oserr,
                       char* dberrstr, char* oserrstr) 
{
    if ((dbproc == nullptr) || (DBDEAD(dbproc))) {
        return INT_CANCEL;
    }
    return INT_CANCEL;
}

static int msg_handler(DBPROCESS* dbproc, DBINT msgno, int msgstate, int severity,
                       char* msgtext, char* srvname, char* procname, int line) {
    return 0;
}

// SybaseConnection implementation
SybaseConnection::SybaseConnection() : login_(nullptr), dbproc_(nullptr) {
    if (!initialized_) {
        if (dbinit() == FAIL) {
            lastError_ = "Failed to initialize DB-Library";
            return;
        }
        dberrhandle(err_handler);
        dbmsghandle(msg_handler);
        initialized_ = true;
    }
}

SybaseConnection::~SybaseConnection() {
    close();
}

bool SybaseConnection::open(const std::string& connectionString) {
    // Parse connection string: "server=host;user=username;password=pwd;database=dbname"
    std::string server, user, password, database;
    
    std::istringstream iss(connectionString);
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
    
    login_ = dblogin();
    if (!login_) {
        lastError_ = "Failed to allocate login structure";
        return false;
    }
    
    DBSETLUSER(login_, user.c_str());
    DBSETLPWD(login_, password.c_str());
    
    dbproc_ = dbopen(login_, server.c_str());
    if (!dbproc_) {
        lastError_ = "Failed to connect to server";
        dbloginfree(login_);
        login_ = nullptr;
        return false;
    }
    
    if (!database.empty()) {
        if (dbuse(dbproc_, database.c_str()) == FAIL) {
            lastError_ = "Failed to use database";
            dbclose(dbproc_);
            dbloginfree(login_);
            dbproc_ = nullptr;
            login_ = nullptr;
            return false;
        }
    }
    
    return true;
}

void SybaseConnection::close() {
    if (dbproc_) {
        dbclose(dbproc_);
        dbproc_ = nullptr;
    }
    if (login_) {
        dbloginfree(login_);
        login_ = nullptr;
    }
}

bool SybaseConnection::isOpen() const {
    return dbproc_ != nullptr && !DBDEAD(dbproc_);
}

std::shared_ptr<IStatement> SybaseConnection::createStatement(const std::string& sql) {
    return std::make_shared<SybaseStatement>(this, sql);
}

std::shared_ptr<ITransaction> SybaseConnection::beginTransaction() {
    return std::make_shared<SybaseTransaction>(this);
}

bool SybaseConnection::execute(const std::string& sql) {
    if (dbcmd(dbproc_, sql.c_str()) == FAIL) {
        lastError_ = "Failed to set command";
        return false;
    }
    
    if (dbsqlexec(dbproc_) == FAIL) {
        lastError_ = "Failed to execute command";
        return false;
    }
    
    // Process results
    RETCODE ret;
    while ((ret = dbresults(dbproc_)) != NO_MORE_RESULTS) {
        if (ret == FAIL) {
            lastError_ = "Failed to process results";
            return false;
        }
    }
    
    return true;
}

std::string SybaseConnection::getLastError() const {
    return lastError_;
}

// SybaseStatement implementation
SybaseStatement::SybaseStatement(SybaseConnection* conn, const std::string& sql)
    : conn_(conn), sql_(sql) {}

SybaseStatement::~SybaseStatement() {}

void SybaseStatement::bindInt(int index, int32_t value) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
    }
    paramValues_[index - 1] = std::to_string(value);
}

void SybaseStatement::bindLong(int index, int64_t value) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
    }
    paramValues_[index - 1] = std::to_string(value);
}

void SybaseStatement::bindDouble(int index, double value) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
    }
    paramValues_[index - 1] = std::to_string(value);
}

void SybaseStatement::bindString(int index, const std::string& value) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
    }
    // Escape single quotes
    std::string escaped = value;
    size_t pos = 0;
    while ((pos = escaped.find('\'', pos)) != std::string::npos) {
        escaped.insert(pos, "'");
        pos += 2;
    }
    paramValues_[index - 1] = "'" + escaped + "'";
}

void SybaseStatement::bindNull(int index) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
    }
    paramValues_[index - 1] = "NULL";
}

std::string SybaseStatement::buildSQL() {
    std::string result = sql_;
    int paramIndex = 1;
    
    for (const auto& value : paramValues_) {
        std::string placeholder = "$" + std::to_string(paramIndex);
        size_t pos = result.find(placeholder);
        if (pos != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
        }
        paramIndex++;
    }
    
    return result;
}

std::shared_ptr<IResultSet> SybaseStatement::executeQuery() {
    std::string finalSQL = buildSQL();
    
    if (dbcmd(conn_->getHandle(), finalSQL.c_str()) == FAIL) {
        return nullptr;
    }
    
    if (dbsqlexec(conn_->getHandle()) == FAIL) {
        return nullptr;
    }
    
    if (dbresults(conn_->getHandle()) != SUCCEED) {
        return nullptr;
    }
    
    return std::make_shared<SybaseResultSet>(conn_->getHandle());
}

int SybaseStatement::executeUpdate() {
    std::string finalSQL = buildSQL();
    
    if (dbcmd(conn_->getHandle(), finalSQL.c_str()) == FAIL) {
        return -1;
    }
    
    if (dbsqlexec(conn_->getHandle()) == FAIL) {
        return -1;
    }
    
    RETCODE ret = dbresults(conn_->getHandle());
    if (ret == FAIL) {
        return -1;
    }
    
    return static_cast<int>(dbcount(conn_->getHandle()));
}

void SybaseStatement::reset() {
    paramValues_.clear();
}

// SybaseResultSet implementation
SybaseResultSet::SybaseResultSet(DBPROCESS* dbproc)
    : dbproc_(dbproc), hasRows_(false) {
    columnCount_ = dbnumcols(dbproc_);
}

SybaseResultSet::~SybaseResultSet() {}

bool SybaseResultSet::next() {
    RETCODE ret = dbnextrow(dbproc_);
    hasRows_ = (ret == REG_ROW);
    return hasRows_;
}

int32_t SybaseResultSet::getInt(int index) const {
    DBINT value;
    dbconvert(dbproc_, dbcoltype(dbproc_, index + 1), dbdata(dbproc_, index + 1),
              dbdatlen(dbproc_, index + 1), SYBINT4, (BYTE*)&value, sizeof(value));
    return static_cast<int32_t>(value);
}

int64_t SybaseResultSet::getLong(int index) const {
    DBBIGINT value;
    dbconvert(dbproc_, dbcoltype(dbproc_, index + 1), dbdata(dbproc_, index + 1),
              dbdatlen(dbproc_, index + 1), SYBINT8, (BYTE*)&value, sizeof(value));
    return static_cast<int64_t>(value);
}

double SybaseResultSet::getDouble(int index) const {
    DBFLT8 value;
    dbconvert(dbproc_, dbcoltype(dbproc_, index + 1), dbdata(dbproc_, index + 1),
              dbdatlen(dbproc_, index + 1), SYBFLT8, (BYTE*)&value, sizeof(value));
    return static_cast<double>(value);
}

std::string SybaseResultSet::getString(int index) const {
    BYTE* data = dbdata(dbproc_, index + 1);
    if (!data) return "";
    
    DBINT len = dbdatlen(dbproc_, index + 1);
    return std::string(reinterpret_cast<char*>(data), len);
}

bool SybaseResultSet::isNull(int index) const {
    return dbdata(dbproc_, index + 1) == nullptr;
}

int SybaseResultSet::getColumnCount() const {
    return columnCount_;
}

std::string SybaseResultSet::getColumnName(int index) const {
    return dbcolname(dbproc_, index + 1);
}

// SybaseTransaction implementation
SybaseTransaction::SybaseTransaction(SybaseConnection* conn)
    : conn_(conn), active_(true) {
    conn_->execute("BEGIN TRANSACTION");
}

SybaseTransaction::~SybaseTransaction() {
    if (active_) {
        rollback();
    }
}

bool SybaseTransaction::commit() {
    if (!active_) return false;
    
    bool result = conn_->execute("COMMIT TRANSACTION");
    active_ = false;
    return result;
}

bool SybaseTransaction::rollback() {
    if (!active_) return false;
    
    bool result = conn_->execute("ROLLBACK TRANSACTION");
    active_ = false;
    return result;
}

bool SybaseTransaction::isActive() const {
    return active_;
}

} // namespace db
} // namespace hft

#endif