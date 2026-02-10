#include "sybase/SybValue.hpp"
#include "db/DBException.hpp"
#include <cstdlib>

#ifdef WITH_SYBASE

SybValue::SybValue() = default;

SybValue::SybValue(std::string v, bool isNull)
    : _value(std::move(v)), _null(isNull) {}

bool SybValue::isNull() const {
    return _null;
}

int SybValue::asInt() const {
    if (_null) throw DBException("SybValue::asInt: null");
    return std::stoi(_value);
}

double SybValue::asDouble() const {
    if (_null) throw DBException("SybValue::asDouble: null");
    return std::stod(_value);
}

std::string SybValue::asString() const {
    if (_null) return {};
    return _value;
}

#endif
