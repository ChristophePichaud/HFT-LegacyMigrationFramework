#pragma once

#include "IConnection.h"
#include "IStatement.h"
#include "IResultSet.h"
#include "ITransaction.h"
#ifdef WITH_SYBASE
#include <sybfront.h>
#include <sybdb.h>
#endif
#include <memory>
#include <string>

#ifdef WITH_SYBASE

namespace hft {
namespace db {

class SybaseStatement;
class SybaseTransaction;

/**
 * @brief Sybase DB-Lib connection implementation
 */
class SybaseConnection : public IConnection {
public:
    SybaseConnection();
    ~SybaseConnection() override;

    bool open(const std::string& connectionString) override;
    void close() override;
    bool isOpen() const override;
    std::shared_ptr<IStatement> createStatement(const std::string& sql) override;
    std::shared_ptr<ITransaction> beginTransaction() override;
    bool execute(const std::string& sql) override;
    std::string getLastError() const override;

    DBPROCESS* getHandle() { return dbproc_; }

private:
    LOGINREC* login_;
    DBPROCESS* dbproc_;
    std::string lastError_;
    static bool initialized_;
};

/**
 * @brief Sybase DB-Lib prepared statement implementation
 */
class SybaseStatement : public IStatement {
public:
    SybaseStatement(SybaseConnection* conn, const std::string& sql);
    ~SybaseStatement() override;

    void bindInt(int index, int32_t value) override;
    void bindLong(int index, int64_t value) override;
    void bindDouble(int index, double value) override;
    void bindString(int index, const std::string& value) override;
    void bindNull(int index) override;
    std::shared_ptr<IResultSet> executeQuery() override;
    int executeUpdate() override;
    void reset() override;

private:
    std::string buildSQL();
    
    SybaseConnection* conn_;
    std::string sql_;
    std::vector<std::string> paramValues_;
};

/**
 * @brief Sybase DB-Lib result set implementation
 */
class SybaseResultSet : public IResultSet {
public:
    SybaseResultSet(DBPROCESS* dbproc);
    ~SybaseResultSet() override;

    bool next() override;
    int32_t getInt(int index) const override;
    int64_t getLong(int index) const override;
    double getDouble(int index) const override;
    std::string getString(int index) const override;
    bool isNull(int index) const override;
    int getColumnCount() const override;
    std::string getColumnName(int index) const override;

private:
    DBPROCESS* dbproc_;
    int columnCount_;
    bool hasRows_;
};

/**
 * @brief Sybase DB-Lib transaction implementation
 */
class SybaseTransaction : public ITransaction {
public:
    SybaseTransaction(SybaseConnection* conn);
    ~SybaseTransaction() override;

    bool commit() override;
    bool rollback() override;
    bool isActive() const override;

private:
    SybaseConnection* conn_;
    bool active_;
};

} // namespace db
} // namespace hft
#endif