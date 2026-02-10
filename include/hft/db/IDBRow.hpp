#pragma once
#include <cstddef>

class IDBValue;

class IDBRow {
public:
    virtual ~IDBRow() = default;
    virtual std::size_t columnCount() const = 0;
    virtual const IDBValue& operator[](std::size_t idx) const = 0;
};
