#pragma once
#include "db/IDBConnection.hpp"
#include <string>
#include <memory>

class MockReader;
class MockPreparedStatement;
class MockTransaction;

class MockConnection : public IDBConnection {
public:
    MockConnection() = default;

    std::unique_ptr<IDBReader> executeQuery(const std::string& sql) override;
    std::unique_ptr<IDBPreparedStatement> prepare(const std::string& sql) override;
    std::unique_ptr<IDBTransaction> beginTransaction() override;

    const std::string& lastQuery() const { return _lastQuery; }
    const std::string& lastPreparedSQL() const { return _lastPreparedSQL; }

private:
    std::string _lastQuery;
    std::string _lastPreparedSQL;
};
