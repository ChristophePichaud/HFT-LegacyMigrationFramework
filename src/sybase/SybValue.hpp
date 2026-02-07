#pragma once
#include "db/IDBValue.hpp"
#include <string>

class SybValue : public IDBValue {
public:
    SybValue();
    SybValue(std::string v, bool isNull);

    bool isNull() const override;
    int asInt() const override;
    double asDouble() const override;
    std::string asString() const override;

private:
    std::string _value;
    bool _null{true};
};
