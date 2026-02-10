#pragma once
#include <nlohmann/json.hpp>

class BaseEntity {
public:
    virtual ~BaseEntity() = default;
    virtual nlohmann::json toJson() const = 0;
};
