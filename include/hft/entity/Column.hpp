#pragma once
#include <string_view>

template<typename Entity, typename FieldType>
struct Column {
    std::string_view name;
    FieldType Entity::* member;
};
