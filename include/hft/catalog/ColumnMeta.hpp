#pragma once
#include <string>

struct ColumnMeta {
    std::string name;
    std::string typeName;
    int length{};
    int scale{};
    bool nullable{};
};
