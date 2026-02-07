#pragma once
#include "db/IDBTransaction.hpp"

class MockTransaction : public IDBTransaction {
public:
    MockTransaction() = default;

    void commit() override { _committed = true; }
    void rollback() override { _rolledBack = true; }

    bool committed() const { return _committed; }
    bool rolledBack() const { return _rolledBack; }

private:
    bool _committed{false};
    bool _rolledBack{false};
};
