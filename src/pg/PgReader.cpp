#include "PgReader.hpp"
#include "pg/PgRow.hpp"
#include "db/DBException.hpp"

#ifdef WITH_POSTGRESQL
#include <libpq-fe.h>
#endif

PgReader::PgReader(PGresult* result)
    : _result(result) {
#ifdef WITH_POSTGRESQL
    if (_result) {
        _numRows = PQntuples(_result);
    }
#else
    (void)result;
    throw DBException("PostgreSQL support not compiled in");
#endif
}

PgReader::~PgReader() {
#ifdef WITH_POSTGRESQL
    if (_result) {
        PQclear(_result);
        _result = nullptr;
    }
#endif
}

bool PgReader::next() {
#ifdef WITH_POSTGRESQL
    _currentRow++;
    if (_currentRow >= _numRows) {
        return false;
    }
    _row = std::make_unique<PgRow>(_result, _currentRow);
    return true;
#else
    throw DBException("PostgreSQL support not compiled in");
#endif
}

IDBRow& PgReader::row() {
    if (!_row) {
        throw DBException("PgReader::row: no row");
    }
    return *_row;
}
