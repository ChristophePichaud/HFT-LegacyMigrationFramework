#pragma once
#include "db/IDBRow.hpp"
#include <vector>
#include <memory>

// Forward declare libpq type
struct pg_result;
typedef struct pg_result PGresult;

class IDBValue;

class PgRow : public IDBRow {
public:
    PgRow(PGresult* result, int rowNum);
    ~PgRow() override;

    std::size_t columnCount() const override;
    const IDBValue& operator[](std::size_t idx) const override;

private:
    std::vector<std::unique_ptr<IDBValue>> _values;
};
