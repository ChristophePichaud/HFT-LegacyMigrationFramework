#pragma once
#include "db/IDBReader.hpp"
#include <memory>

class IDBRow;

class PgReader : public IDBReader {
public:
    PgReader();
    ~PgReader() override;

    bool next() override;
    IDBRow& row() override;

private:
    std::unique_ptr<IDBRow> _row;
};
