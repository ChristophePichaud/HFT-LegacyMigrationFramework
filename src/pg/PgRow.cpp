#include "pg/PgRow.hpp"
#include "pg/PgValue.hpp"
#include "db/DBException.hpp"

#ifdef WITH_POSTGRESQL
#include <libpq-fe.h>
#endif

PgRow::PgRow(PGresult* result, int rowNum) {
#ifdef WITH_POSTGRESQL
    if (!result) {
        throw DBException("PgRow: result is null");
    }
    
    int numCols = PQnfields(result);
    _values.reserve(numCols);
    
    for (int col = 0; col < numCols; ++col) {
        bool isNull = PQgetisnull(result, rowNum, col) != 0;
        std::string value;
        if (!isNull) {
            char* val = PQgetvalue(result, rowNum, col);
            value = val ? val : "";
        }
        _values.push_back(std::make_unique<PgValue>(value, isNull));
    }
#else
    (void)result;
    (void)rowNum;
    throw DBException("PostgreSQL support not compiled in");
#endif
}

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
