#pragma once
#include "db/IDBTransaction.hpp"

class SybTransaction : public IDBTransaction {
public:
    SybTransaction();
    ~SybTransaction() override;

    void commit() override;
    void rollback() override;

private:
    bool _active{true};
};
