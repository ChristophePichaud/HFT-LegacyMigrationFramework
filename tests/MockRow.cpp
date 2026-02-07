#include "MockRow.hpp"
#include "db/IDBValue.hpp"
#include "db/DBException.hpp"

class SimpleValue : public IDBValue {
public:
    SimpleValue(int v) : _v(std::to_string(v)), _null(false) {}
    bool isNull() const override { return _null; }
    int asInt() const override { return std::stoi(_v); }
    double asDouble() const override { return std::stod(_v); }
    std::string asString() const override { return _v; }
private:
    std::string _v;
    bool _null{false};
};

MockRow::MockRow() {
    _values.emplace_back(std::make_unique<SimpleValue>(42));
}

MockRow::~MockRow() = default;

std::size_t MockRow::columnCount() const {
    return _values.size();
}

const IDBValue& MockRow::operator[](std::size_t idx) const {
    if (idx >= _values.size()) {
        throw DBException("MockRow::operator[]: index out of range");
    }
    return *_values[idx];
}
