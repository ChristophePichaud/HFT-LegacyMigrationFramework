#pragma once
#include "db/IDBTransaction.hpp"

class PgTransaction : public IDBTransaction {
public:
    PgTransaction();
    ~PgTransaction() override;

    void commit() override;
    void rollback() override;

private:
    bool _active{true};
};
