#pragma once
#include "db/IDBTransaction.hpp"

// Forward declaration
class PgConnection;

class PgTransaction : public IDBTransaction {
public:
    explicit PgTransaction(PgConnection* conn);
    ~PgTransaction() override;

    void commit() override;
    void rollback() override;

private:
    PgConnection* _conn{nullptr};
    bool _active{true};
};
