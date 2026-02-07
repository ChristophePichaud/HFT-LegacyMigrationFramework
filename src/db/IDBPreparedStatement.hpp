#pragma once
#include <memory>
#include <string>

class IDBReader;

class IDBPreparedStatement {
public:
    virtual ~IDBPreparedStatement() = default;

    virtual void bindInt(int index, int value) = 0;
    virtual void bindDouble(int index, double value) = 0;
    virtual void bindString(int index, const std::string& value) = 0;

    virtual std::unique_ptr<IDBReader> executeQuery() = 0;
    virtual void executeUpdate() = 0;
};
