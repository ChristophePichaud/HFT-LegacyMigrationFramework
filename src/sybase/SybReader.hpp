#pragma once
#include "db/IDBReader.hpp"
#include <memory>

// Forward declare DB-Lib type
struct tds_dbproc;
typedef struct tds_dbproc DBPROCESS;

class IDBRow;

class SybReader : public IDBReader {
public:
    explicit SybReader(DBPROCESS* dbproc);
    ~SybReader() override;

    bool next() override;
    IDBRow& row() override;

private:
    DBPROCESS* _dbproc{nullptr};
    std::unique_ptr<IDBRow> _row;
};
