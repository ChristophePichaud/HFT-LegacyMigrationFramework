#pragma once

class IDBTransaction {
public:
    virtual ~IDBTransaction() = default;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};
