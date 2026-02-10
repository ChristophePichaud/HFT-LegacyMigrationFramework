#pragma once
#include "db/IDBReader.hpp"
#include <memory>

// Forward declare libpq type
struct pg_result;
typedef struct pg_result PGresult;

class IDBRow;

class PgReader : public IDBReader {
public:
    explicit PgReader(PGresult* result);
    ~PgReader() override;

    bool next() override;
    IDBRow& row() override;

private:
    PGresult* _result{nullptr};
    int _currentRow{-1};
    int _numRows{0};
    std::unique_ptr<IDBRow> _row;
};
