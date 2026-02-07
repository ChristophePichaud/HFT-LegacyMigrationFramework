#include "MockConnection.hpp"
#include "MockReader.hpp"
#include "MockPreparedStatement.hpp"
#include "MockTransaction.hpp"

std::unique_ptr<IDBReader> MockConnection::executeQuery(const std::string& sql) {
    _lastQuery = sql;
    return std::make_unique<MockReader>();
}

std::unique_ptr<IDBPreparedStatement> MockConnection::prepare(const std::string& sql) {
    _lastPreparedSQL = sql;
    return std::make_unique<MockPreparedStatement>();
}

std::unique_ptr<IDBTransaction> MockConnection::beginTransaction() {
    return std::make_unique<MockTransaction>();
}
