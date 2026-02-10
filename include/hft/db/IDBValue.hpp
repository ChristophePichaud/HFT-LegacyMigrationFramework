#pragma once
#include <string>

class IDBValue {
public:
    virtual ~IDBValue() = default;
    virtual bool isNull() const = 0;
    virtual int asInt() const = 0;
    virtual double asDouble() const = 0;
    virtual std::string asString() const = 0;
};
