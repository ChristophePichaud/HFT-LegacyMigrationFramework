#pragma once

#include "hft/db/IConnection.h"
#include "hft/db/IStatement.h"
#include "hft/reflection/EntityTraits.h"
#include <memory>
#include <vector>
#include <optional>
#include <sstream>

namespace hft {
namespace orm {

/**
 * @brief Generic repository for entity CRUD operations
 */
template<typename T>
class Repository {
public:
    explicit Repository(std::shared_ptr<db::IConnection> connection)
        : connection_(connection) {}
    
    /**
     * @brief Find entity by ID
     */
    std::optional<T> findById(int64_t id) {
        std::string sql = "SELECT * FROM " + std::string(reflection::EntityTraits<T>::tableName()) + " WHERE id = $1";
        
        auto stmt = connection_->createStatement(sql);
        stmt->bindLong(1, id);
        
        auto result = stmt->executeQuery();
        if (!result || !result->next()) {
            return std::nullopt;
        }
        
        return mapResultToEntity(result);
    }
    
    /**
     * @brief Find all entities
     */
    std::vector<T> findAll() {
        std::string sql = "SELECT * FROM " + std::string(reflection::EntityTraits<T>::tableName());
        
        auto stmt = connection_->createStatement(sql);
        auto result = stmt->executeQuery();
        
        std::vector<T> entities;
        if (result) {
            while (result->next()) {
                entities.push_back(mapResultToEntity(result).value());
            }
        }
        
        return entities;
    }
    
    /**
     * @brief Insert a new entity
     */
    bool insert(T& entity) {
        std::ostringstream sql;
        sql << "INSERT INTO " << reflection::EntityTraits<T>::tableName() << " (";
        
        std::vector<std::string> columns;
        std::vector<std::string> values;
        int paramIndex = 1;
        
        reflection::EntityTraits<T>::forEachField(entity, [&](const std::string& name, 
                                                                reflection::FieldType type,
                                                                auto* ptr,
                                                                bool isPK,
                                                                bool isNullable) {
            if (!isPK) { // Skip auto-increment primary key
                columns.push_back(name);
                values.push_back("$" + std::to_string(paramIndex++));
            }
        });
        
        for (size_t i = 0; i < columns.size(); ++i) {
            sql << columns[i];
            if (i < columns.size() - 1) sql << ", ";
        }
        
        sql << ") VALUES (";
        
        for (size_t i = 0; i < values.size(); ++i) {
            sql << values[i];
            if (i < values.size() - 1) sql << ", ";
        }
        
        sql << ")";
        
        auto stmt = connection_->createStatement(sql.str());
        bindEntityToStatement(entity, stmt, true);
        
        return stmt->executeUpdate() > 0;
    }
    
    /**
     * @brief Update an existing entity
     */
    bool update(const T& entity) {
        std::ostringstream sql;
        sql << "UPDATE " << reflection::EntityTraits<T>::tableName() << " SET ";
        
        std::vector<std::string> updates;
        int paramIndex = 1;
        int64_t idValue = 0;
        
        reflection::EntityTraits<T>::forEachField(const_cast<T&>(entity), [&](const std::string& name, 
                                                                                reflection::FieldType type,
                                                                                auto* ptr,
                                                                                bool isPK,
                                                                                bool isNullable) {
            if (isPK) {
                idValue = *reinterpret_cast<int64_t*>(ptr);
            } else {
                updates.push_back(name + " = $" + std::to_string(paramIndex++));
            }
        });
        
        for (size_t i = 0; i < updates.size(); ++i) {
            sql << updates[i];
            if (i < updates.size() - 1) sql << ", ";
        }
        
        sql << " WHERE id = $" << paramIndex;
        
        auto stmt = connection_->createStatement(sql.str());
        bindEntityToStatement(entity, stmt, true);
        stmt->bindLong(paramIndex, idValue);
        
        return stmt->executeUpdate() > 0;
    }
    
    /**
     * @brief Delete an entity by ID
     */
    bool deleteById(int64_t id) {
        std::string sql = "DELETE FROM " + std::string(reflection::EntityTraits<T>::tableName()) + " WHERE id = $1";
        
        auto stmt = connection_->createStatement(sql);
        stmt->bindLong(1, id);
        
        return stmt->executeUpdate() > 0;
    }
    
    /**
     * @brief Delete all entities
     */
    int deleteAll() {
        std::string sql = "DELETE FROM " + std::string(reflection::EntityTraits<T>::tableName());
        
        auto stmt = connection_->createStatement(sql);
        return stmt->executeUpdate();
    }

private:
    std::optional<T> mapResultToEntity(std::shared_ptr<db::IResultSet> result) {
        T entity{};
        int colIndex = 0;
        
        reflection::EntityTraits<T>::forEachField(entity, [&](const std::string& name, 
                                                               reflection::FieldType type,
                                                               auto* ptr,
                                                               bool isPK,
                                                               bool isNullable) {
            if (!result->isNull(colIndex)) {
                using FieldType = std::remove_pointer_t<decltype(ptr)>;
                
                if constexpr (std::is_same_v<FieldType, int32_t> || std::is_same_v<FieldType, int>) {
                    *ptr = result->getInt(colIndex);
                } else if constexpr (std::is_same_v<FieldType, int64_t> || std::is_same_v<FieldType, long> || std::is_same_v<FieldType, long long>) {
                    *ptr = result->getLong(colIndex);
                } else if constexpr (std::is_same_v<FieldType, double> || std::is_same_v<FieldType, float>) {
                    *ptr = result->getDouble(colIndex);
                } else if constexpr (std::is_same_v<FieldType, std::string>) {
                    *ptr = result->getString(colIndex);
                }
            }
            colIndex++;
        });
        
        return entity;
    }
    
    void bindEntityToStatement(const T& entity, std::shared_ptr<db::IStatement> stmt, bool skipPK) {
        int paramIndex = 1;
        
        reflection::EntityTraits<T>::forEachField(const_cast<T&>(entity), [&](const std::string& name, 
                                                                                reflection::FieldType type,
                                                                                auto* ptr,
                                                                                bool isPK,
                                                                                bool isNullable) {
            if (skipPK && isPK) return;
            
            using FieldType = std::remove_pointer_t<decltype(ptr)>;
            
            if constexpr (std::is_same_v<FieldType, int32_t> || std::is_same_v<FieldType, int>) {
                stmt->bindInt(paramIndex++, *ptr);
            } else if constexpr (std::is_same_v<FieldType, int64_t> || std::is_same_v<FieldType, long> || std::is_same_v<FieldType, long long>) {
                stmt->bindLong(paramIndex++, *ptr);
            } else if constexpr (std::is_same_v<FieldType, double> || std::is_same_v<FieldType, float>) {
                stmt->bindDouble(paramIndex++, *ptr);
            } else if constexpr (std::is_same_v<FieldType, std::string>) {
                stmt->bindString(paramIndex++, *ptr);
            }
        });
    }
    
    std::shared_ptr<db::IConnection> connection_;
};

} // namespace orm
} // namespace hft
