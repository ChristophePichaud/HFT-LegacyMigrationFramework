#include "SybConnection.hpp"
#include "db/DBException.hpp"

SybConnection::SybConnection(const std::string& conninfo)
    : _conninfo(conninfo) {}

SybConnection::~SybConnection() = default;

std::unique_ptr<IDBReader>
SybConnection::executeQuery(const std::string& sql) {
    (void)sql;
    throw DBException("SybConnection::executeQuery not implemented");
}

std::unique_ptr<IDBPreparedStatement>
SybConnection::prepare(const std::string& sql) {
    (void)sql;
    throw DBException("SybConnection::prepare not implemented");
}

std::unique_ptr<IDBTransaction>
SybConnection::beginTransaction() {
    throw DBException("SybConnection::beginTransaction not implemented");
}
