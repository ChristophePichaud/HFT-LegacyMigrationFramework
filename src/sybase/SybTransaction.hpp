#pragma once
#include "db/IDBTransaction.hpp"

// Forward declaration
class SybConnection;

class SybTransaction : public IDBTransaction {
public:
    explicit SybTransaction(SybConnection* conn);
    ~SybTransaction() override;

    void commit() override;
    void rollback() override;

private:
    SybConnection* _conn{nullptr};
    bool _active{true};
};
