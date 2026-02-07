#pragma once

#include "IConnection.h"
#include "IStatement.h"
#include "IResultSet.h"
#include "ITransaction.h"
#include <libpq-fe.h>
#include <memory>
#include <string>

namespace hft {
namespace db {

class PostgreSQLStatement;
class PostgreSQLTransaction;

/**
 * @brief PostgreSQL connection implementation
 */
class PostgreSQLConnection : public IConnection {
public:
    PostgreSQLConnection();
    ~PostgreSQLConnection() override;

    bool open(const std::string& connectionString) override;
    void close() override;
    bool isOpen() const override;
    std::shared_ptr<IStatement> createStatement(const std::string& sql) override;
    std::shared_ptr<ITransaction> beginTransaction() override;
    bool execute(const std::string& sql) override;
    std::string getLastError() const override;

    PGconn* getHandle() { return conn_; }

private:
    PGconn* conn_;
    std::string lastError_;
};

/**
 * @brief PostgreSQL prepared statement implementation
 */
class PostgreSQLStatement : public IStatement {
public:
    PostgreSQLStatement(PostgreSQLConnection* conn, const std::string& sql);
    ~PostgreSQLStatement() override;

    void bindInt(int index, int32_t value) override;
    void bindLong(int index, int64_t value) override;
    void bindDouble(int index, double value) override;
    void bindString(int index, const std::string& value) override;
    void bindNull(int index) override;
    std::shared_ptr<IResultSet> executeQuery() override;
    int executeUpdate() override;
    void reset() override;

private:
    PostgreSQLConnection* conn_;
    std::string sql_;
    std::string stmtName_;
    std::vector<std::string> paramValues_;
    std::vector<int> paramLengths_;
    std::vector<int> paramFormats_;
    std::vector<const char*> paramPointers_;
    bool prepared_;
};

/**
 * @brief PostgreSQL result set implementation
 */
class PostgreSQLResultSet : public IResultSet {
public:
    PostgreSQLResultSet(PGresult* result);
    ~PostgreSQLResultSet() override;

    bool next() override;
    int32_t getInt(int index) const override;
    int64_t getLong(int index) const override;
    double getDouble(int index) const override;
    std::string getString(int index) const override;
    bool isNull(int index) const override;
    int getColumnCount() const override;
    std::string getColumnName(int index) const override;

private:
    PGresult* result_;
    int currentRow_;
    int rowCount_;
};

/**
 * @brief PostgreSQL transaction implementation
 */
class PostgreSQLTransaction : public ITransaction {
public:
    PostgreSQLTransaction(PostgreSQLConnection* conn);
    ~PostgreSQLTransaction() override;

    bool commit() override;
    bool rollback() override;
    bool isActive() const override;

private:
    PostgreSQLConnection* conn_;
    bool active_;
};

} // namespace db
} // namespace hft
