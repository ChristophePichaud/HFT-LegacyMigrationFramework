#include "db/PostgreSQLConnection.h"
#include <cstring>
#include <sstream>
#include <cstdlib>

namespace hft {
namespace db {

static int statementCounter = 0;

// PostgreSQLConnection implementation
PostgreSQLConnection::PostgreSQLConnection() : conn_(nullptr) {}

PostgreSQLConnection::~PostgreSQLConnection() {
    close();
}

bool PostgreSQLConnection::open(const std::string& connectionString) {
    conn_ = PQconnectdb(connectionString.c_str());
    
    if (PQstatus(conn_) != CONNECTION_OK) {
        lastError_ = PQerrorMessage(conn_);
        PQfinish(conn_);
        conn_ = nullptr;
        return false;
    }
    
    return true;
}

void PostgreSQLConnection::close() {
    if (conn_) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
}

bool PostgreSQLConnection::isOpen() const {
    return conn_ != nullptr && PQstatus(conn_) == CONNECTION_OK;
}

std::shared_ptr<IStatement> PostgreSQLConnection::createStatement(const std::string& sql) {
    return std::make_shared<PostgreSQLStatement>(this, sql);
}

std::shared_ptr<ITransaction> PostgreSQLConnection::beginTransaction() {
    return std::make_shared<PostgreSQLTransaction>(this);
}

bool PostgreSQLConnection::execute(const std::string& sql) {
    PGresult* result = PQexec(conn_, sql.c_str());
    ExecStatusType status = PQresultStatus(result);
    
    bool success = (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK);
    
    if (!success) {
        lastError_ = PQerrorMessage(conn_);
    }
    
    PQclear(result);
    return success;
}

std::string PostgreSQLConnection::getLastError() const {
    return lastError_;
}

// PostgreSQLStatement implementation
PostgreSQLStatement::PostgreSQLStatement(PostgreSQLConnection* conn, const std::string& sql)
    : conn_(conn), sql_(sql), prepared_(false) {
    std::ostringstream oss;
    oss << "stmt_" << statementCounter++;
    stmtName_ = oss.str();
}

PostgreSQLStatement::~PostgreSQLStatement() {
    if (prepared_) {
        std::string deallocate = "DEALLOCATE " + stmtName_;
        PQexec(conn_->getHandle(), deallocate.c_str());
    }
}

void PostgreSQLStatement::bindInt(int index, int32_t value) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
        paramLengths_.resize(index);
        paramFormats_.resize(index);
    }
    
    paramValues_[index - 1] = std::to_string(value);
    paramLengths_[index - 1] = 0;
    paramFormats_[index - 1] = 0; // text format
}

void PostgreSQLStatement::bindLong(int index, int64_t value) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
        paramLengths_.resize(index);
        paramFormats_.resize(index);
    }
    
    paramValues_[index - 1] = std::to_string(value);
    paramLengths_[index - 1] = 0;
    paramFormats_[index - 1] = 0;
}

void PostgreSQLStatement::bindDouble(int index, double value) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
        paramLengths_.resize(index);
        paramFormats_.resize(index);
    }
    
    paramValues_[index - 1] = std::to_string(value);
    paramLengths_[index - 1] = 0;
    paramFormats_[index - 1] = 0;
}

void PostgreSQLStatement::bindString(int index, const std::string& value) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
        paramLengths_.resize(index);
        paramFormats_.resize(index);
    }
    
    paramValues_[index - 1] = value;
    paramLengths_[index - 1] = 0;
    paramFormats_[index - 1] = 0;
}

void PostgreSQLStatement::bindNull(int index) {
    if (index > static_cast<int>(paramValues_.size())) {
        paramValues_.resize(index);
        paramLengths_.resize(index);
        paramFormats_.resize(index);
    }
    
    paramValues_[index - 1] = "";
    paramLengths_[index - 1] = 0;
    paramFormats_[index - 1] = 0;
}

std::shared_ptr<IResultSet> PostgreSQLStatement::executeQuery() {
    if (!prepared_) {
        PGresult* prepResult = PQprepare(conn_->getHandle(), stmtName_.c_str(), 
                                         sql_.c_str(), paramValues_.size(), nullptr);
        
        if (PQresultStatus(prepResult) != PGRES_COMMAND_OK) {
            PQclear(prepResult);
            return nullptr;
        }
        
        PQclear(prepResult);
        prepared_ = true;
    }
    
    paramPointers_.clear();
    for (const auto& val : paramValues_) {
        paramPointers_.push_back(val.c_str());
    }
    
    PGresult* result = PQexecPrepared(conn_->getHandle(), stmtName_.c_str(),
                                      paramPointers_.size(), paramPointers_.data(),
                                      paramLengths_.data(), paramFormats_.data(), 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        PQclear(result);
        return nullptr;
    }
    
    return std::make_shared<PostgreSQLResultSet>(result);
}

int PostgreSQLStatement::executeUpdate() {
    if (!prepared_) {
        PGresult* prepResult = PQprepare(conn_->getHandle(), stmtName_.c_str(), 
                                         sql_.c_str(), paramValues_.size(), nullptr);
        
        if (PQresultStatus(prepResult) != PGRES_COMMAND_OK) {
            PQclear(prepResult);
            return -1;
        }
        
        PQclear(prepResult);
        prepared_ = true;
    }
    
    paramPointers_.clear();
    for (const auto& val : paramValues_) {
        paramPointers_.push_back(val.c_str());
    }
    
    PGresult* result = PQexecPrepared(conn_->getHandle(), stmtName_.c_str(),
                                      paramPointers_.size(), paramPointers_.data(),
                                      paramLengths_.data(), paramFormats_.data(), 0);
    
    ExecStatusType status = PQresultStatus(result);
    int affected = 0;
    
    if (status == PGRES_COMMAND_OK) {
        const char* rows = PQcmdTuples(result);
        affected = rows ? std::atoi(rows) : 0;
    }
    
    PQclear(result);
    return affected;
}

void PostgreSQLStatement::reset() {
    paramValues_.clear();
    paramLengths_.clear();
    paramFormats_.clear();
    paramPointers_.clear();
}

// PostgreSQLResultSet implementation
PostgreSQLResultSet::PostgreSQLResultSet(PGresult* result)
    : result_(result), currentRow_(-1) {
    rowCount_ = PQntuples(result);
}

PostgreSQLResultSet::~PostgreSQLResultSet() {
    if (result_) {
        PQclear(result_);
    }
}

bool PostgreSQLResultSet::next() {
    currentRow_++;
    return currentRow_ < rowCount_;
}

int32_t PostgreSQLResultSet::getInt(int index) const {
    if (isNull(index)) return 0;
    const char* value = PQgetvalue(result_, currentRow_, index);
    return std::atoi(value);
}

int64_t PostgreSQLResultSet::getLong(int index) const {
    if (isNull(index)) return 0;
    const char* value = PQgetvalue(result_, currentRow_, index);
    return std::atoll(value);
}

double PostgreSQLResultSet::getDouble(int index) const {
    if (isNull(index)) return 0.0;
    const char* value = PQgetvalue(result_, currentRow_, index);
    return std::atof(value);
}

std::string PostgreSQLResultSet::getString(int index) const {
    if (isNull(index)) return "";
    return PQgetvalue(result_, currentRow_, index);
}

bool PostgreSQLResultSet::isNull(int index) const {
    return PQgetisnull(result_, currentRow_, index) == 1;
}

int PostgreSQLResultSet::getColumnCount() const {
    return PQnfields(result_);
}

std::string PostgreSQLResultSet::getColumnName(int index) const {
    return PQfname(result_, index);
}

// PostgreSQLTransaction implementation
PostgreSQLTransaction::PostgreSQLTransaction(PostgreSQLConnection* conn)
    : conn_(conn), active_(true) {
    conn_->execute("BEGIN");
}

PostgreSQLTransaction::~PostgreSQLTransaction() {
    if (active_) {
        rollback();
    }
}

bool PostgreSQLTransaction::commit() {
    if (!active_) return false;
    
    bool result = conn_->execute("COMMIT");
    active_ = false;
    return result;
}

bool PostgreSQLTransaction::rollback() {
    if (!active_) return false;
    
    bool result = conn_->execute("ROLLBACK");
    active_ = false;
    return result;
}

bool PostgreSQLTransaction::isActive() const {
    return active_;
}

} // namespace db
} // namespace hft
