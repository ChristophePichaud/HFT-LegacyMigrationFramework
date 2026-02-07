#include "PgReader.hpp"
#include "pg/PgRow.hpp"
#include "db/DBException.hpp"

PgReader::PgReader()
    : _row(std::make_unique<PgRow>()) {}

PgReader::~PgReader() = default;

bool PgReader::next() {
    // Stub: no rows
    return false;
}

IDBRow& PgReader::row() {
    if (!_row) {
        throw DBException("PgReader::row: no row");
    }
    return *_row;
}
