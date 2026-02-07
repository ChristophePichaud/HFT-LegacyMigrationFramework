#pragma once
#include "db/IDBReader.hpp"
#include <memory>

class IDBRow;

class MockReader : public IDBReader {
public:
    MockReader();
    ~MockReader() override;

    bool next() override;
    IDBRow& row() override;

private:
    std::unique_ptr<IDBRow> _row;
    bool _done{false};
};
