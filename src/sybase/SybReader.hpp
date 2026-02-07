#pragma once
#include "db/IDBReader.hpp"
#include <memory>

class IDBRow;

class SybReader : public IDBReader {
public:
    SybReader();
    ~SybReader() override;

    bool next() override;
    IDBRow& row() override;

private:
    std::unique_ptr<IDBRow> _row;
};
