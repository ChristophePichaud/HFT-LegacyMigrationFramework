#include "PgValue.hpp"
#include "db/DBException.hpp"
#include <cstdlib>

PgValue::PgValue() = default;

PgValue::PgValue(std::string v, bool isNull)
    : _value(std::move(v)), _null(isNull) {}

bool PgValue::isNull() const {
    return _null;
}

int PgValue::asInt() const {
    if (_null) throw DBException("PgValue::asInt: null");
    return std::stoi(_value);
}

double PgValue::asDouble() const {
    if (_null) throw DBException("PgValue::asDouble: null");
    return std::stod(_value);
}

std::string PgValue::asString() const {
    if (_null) return {};
    return _value;
}
