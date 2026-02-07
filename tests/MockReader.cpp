#include "MockReader.hpp"
#include "MockRow.hpp"

MockReader::MockReader()
    : _row(std::make_unique<MockRow>()) {}

MockReader::~MockReader() = default;

bool MockReader::next() {
    if (_done) return false;
    _done = true;
    return true;
}

IDBRow& MockReader::row() {
    return *_row;
}
