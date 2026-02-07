#pragma once
#include <string>
#include <vector>
#include "ColumnMeta.hpp"

struct TableMeta {
    std::string name;
    std::vector<ColumnMeta> columns;
};
