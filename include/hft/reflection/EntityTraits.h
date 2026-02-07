#pragma once

#include <string>
#include <tuple>
#include <type_traits>

namespace hft {
namespace reflection {

/**
 * @brief Type enumeration for entity fields
 */
enum class FieldType {
    INT32,
    INT64,
    DOUBLE,
    STRING,
    BOOL
};

/**
 * @brief Field metadata
 */
template<typename T>
struct FieldInfo {
    std::string name;
    FieldType type;
    T* ptr;
    bool primaryKey;
    bool nullable;
    
    FieldInfo(const std::string& n, FieldType t, T* p, bool pk = false, bool null = false)
        : name(n), type(t), ptr(p), primaryKey(pk), nullable(null) {}
};

/**
 * @brief Base trait for entities - specialize this for your entities
 */
template<typename T>
struct EntityTraits {
    static constexpr const char* tableName() { return "unknown"; }
    static constexpr size_t fieldCount() { return 0; }
};

/**
 * @brief Helper to get field type from C++ type
 */
template<typename T>
constexpr FieldType getFieldType() {
    if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, int>) {
        return FieldType::INT32;
    } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, long> || std::is_same_v<T, long long>) {
        return FieldType::INT64;
    } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
        return FieldType::DOUBLE;
    } else if constexpr (std::is_same_v<T, std::string>) {
        return FieldType::STRING;
    } else if constexpr (std::is_same_v<T, bool>) {
        return FieldType::BOOL;
    } else {
        return FieldType::STRING; // fallback
    }
}

/**
 * @brief Macro to define entity traits
 * 
 * Usage:
 * DEFINE_ENTITY_TRAITS(User, "users",
 *     FIELD(id, INT32, true, false),
 *     FIELD(name, STRING, false, false),
 *     FIELD(email, STRING, false, false)
 * )
 */
#define DEFINE_ENTITY_TRAITS(EntityType, TableName, ...) \
namespace hft { namespace reflection { \
template<> \
struct EntityTraits<EntityType> { \
    static constexpr const char* tableName() { return TableName; } \
    static constexpr size_t fieldCount() { return FIELD_COUNT(__VA_ARGS__); } \
    \
    template<typename Func> \
    static void forEachField(EntityType& entity, Func&& func) { \
        APPLY_FIELDS(entity, func, __VA_ARGS__) \
    } \
}; \
}}

// Helper macros for field counting and iteration
#define FIELD_COUNT(...) (sizeof((int[]){0, ##__VA_ARGS__}) / sizeof(int) - 1)

#define FIELD(Name, Type, PrimaryKey, Nullable) \
    func(#Name, getFieldType<decltype(entity.Name)>(), &entity.Name, PrimaryKey, Nullable);

#define APPLY_FIELDS(entity, func, ...) \
    __VA_ARGS__

/**
 * @brief Simplified macro for defining entity traits
 */
#define ENTITY_FIELD(entity, name, isPK) \
    hft::reflection::FieldInfo(#name, hft::reflection::getFieldType<decltype(entity.name)>(), &entity.name, isPK, false)

} // namespace reflection
} // namespace hft
