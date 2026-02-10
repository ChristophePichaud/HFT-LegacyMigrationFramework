#pragma once
#include "db/IDBTransaction.hpp"

#ifdef WITH_SYBASE

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


#endif
