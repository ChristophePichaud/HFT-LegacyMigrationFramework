#pragma once
#include "db/IDBRow.hpp"
#include <vector>
#include <memory>

#ifdef WITH_SYBASE

// Forward declare DB-Lib type
struct tds_dbproc;
typedef struct tds_dbproc DBPROCESS;

class IDBValue;

class SybRow : public IDBRow {
public:
    explicit SybRow(DBPROCESS* dbproc);
    ~SybRow() override;

    std::size_t columnCount() const override;
    const IDBValue& operator[](std::size_t idx) const override;

private:
    std::vector<std::unique_ptr<IDBValue>> _values;
};

#endif
