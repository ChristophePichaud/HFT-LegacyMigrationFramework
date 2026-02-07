#include "SybReader.hpp"
#include "sybase/SybRow.hpp"
#include "db/DBException.hpp"

SybReader::SybReader()
    : _row(std::make_unique<SybRow>()) {}

SybReader::~SybReader() = default;

bool SybReader::next() {
    return false;
}

IDBRow& SybReader::row() {
    if (!_row) throw DBException("SybReader::row: no row");
    return *_row;
}
