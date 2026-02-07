#include "PgRow.hpp"
#include "pg/PgValue.hpp"
#include "db/DBException.hpp"

PgRow::PgRow() = default;
PgRow::~PgRow() = default;

std::size_t PgRow::columnCount() const {
    return _values.size();
}

const IDBValue& PgRow::operator[](std::size_t idx) const {
    if (idx >= _values.size()) {
        throw DBException("PgRow::operator[]: index out of range");
    }
    return *_values[idx];
}
