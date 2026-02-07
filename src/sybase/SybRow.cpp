#include "SybRow.hpp"
#include "sybase/SybValue.hpp"
#include "db/DBException.hpp"

SybRow::SybRow() = default;
SybRow::~SybRow() = default;

std::size_t SybRow::columnCount() const {
    return _values.size();
}

const IDBValue& SybRow::operator[](std::size_t idx) const {
    if (idx >= _values.size()) {
        throw DBException("SybRow::operator[]: index out of range");
    }
    return *_values[idx];
}
