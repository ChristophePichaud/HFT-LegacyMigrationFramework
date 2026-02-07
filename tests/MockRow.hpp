#pragma once
#include "db/IDBRow.hpp"
#include <vector>
#include <memory>

class IDBValue;

class MockRow : public IDBRow {
public:
    MockRow();
    ~MockRow() override;

    std::size_t columnCount() const override;
    const IDBValue& operator[](std::size_t idx) const override;

private:
    std::vector<std::unique_ptr<IDBValue>> _values;
};
