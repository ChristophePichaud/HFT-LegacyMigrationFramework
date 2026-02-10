#pragma once
#include <memory>
#include <string>

class IDBReader;
class IDBPreparedStatement;
class IDBTransaction;

class IDBConnection {
public:
    virtual ~IDBConnection() = default;

    virtual std::unique_ptr<IDBReader>
    executeQuery(const std::string& sql) = 0;

    virtual std::unique_ptr<IDBPreparedStatement>
    prepare(const std::string& sql) = 0;

    virtual std::unique_ptr<IDBTransaction>
    beginTransaction() = 0;
};
