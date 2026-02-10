#pragma once
#include "entity/EntityTraits.hpp"
#include "db/IDBConnection.hpp"
#include "db/IDBPreparedStatement.hpp"
#include "db/IDBReader.hpp"
#include "db/IDBRow.hpp"
#include "db/IDBValue.hpp"
#include "db/IDBTransaction.hpp"
#include "db/DBException.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <type_traits>

template<typename Entity>
class Repository {
public:
    explicit Repository(IDBConnection& conn)
        : _conn(conn) {}

    std::vector<Entity> getAll() {
        std::vector<Entity> result;
        std::ostringstream oss;
        oss << "SELECT * FROM " << EntityTraits<Entity>::tableName;
        auto reader = _conn.executeQuery(oss.str());
        while (reader->next()) {
            Entity e{};
            mapRowToEntity(reader->row(), e);
            result.push_back(std::move(e));
        }
        return result;
    }

    Entity getById(int id) {
        std::ostringstream oss;
        oss << "SELECT * FROM " << EntityTraits<Entity>::tableName
            << " WHERE " << EntityTraits<Entity>::primaryKey << "=" << id;
        auto reader = _conn.executeQuery(oss.str());
        if (!reader->next()) {
            throw DBException("Entity not found");
        }
        Entity e{};
        mapRowToEntity(reader->row(), e);
        return e;
    }

    void insert(const Entity& e) {
        std::ostringstream oss;
        oss << "INSERT INTO " << EntityTraits<Entity>::tableName << " (";
        
        // Build column list (skip primary key if auto-increment)
        bool first = true;
        std::apply([&](auto&&... col) {
            ((buildColumnList(oss, col, first, true)), ...);
        }, EntityTraits<Entity>::columns);
        
        oss << ") VALUES (";
        
        // Build values list
        first = true;
        std::apply([&](auto&&... col) {
            ((buildValuesList(oss, col, e, first, true)), ...);
        }, EntityTraits<Entity>::columns);
        
        oss << ")";
        
        _conn.executeQuery(oss.str());
    }

    void update(const Entity& e) {
        std::ostringstream oss;
        oss << "UPDATE " << EntityTraits<Entity>::tableName << " SET ";
        
        // Build SET clause (skip primary key)
        bool first = true;
        std::apply([&](auto&&... col) {
            ((buildUpdateSet(oss, col, e, first)), ...);
        }, EntityTraits<Entity>::columns);
        
        // Add WHERE clause with primary key
        oss << " WHERE " << EntityTraits<Entity>::primaryKey << "=";
        std::apply([&](auto&&... col) {
            ((buildWhereClause(oss, col, e)), ...);
        }, EntityTraits<Entity>::columns);
        
        _conn.executeQuery(oss.str());
    }

    void remove(const Entity& e) {
        std::ostringstream oss;
        oss << "DELETE FROM " << EntityTraits<Entity>::tableName 
            << " WHERE " << EntityTraits<Entity>::primaryKey << "=";
        
        // Get primary key value
        std::apply([&](auto&&... col) {
            ((buildWhereClause(oss, col, e)), ...);
        }, EntityTraits<Entity>::columns);
        
        _conn.executeQuery(oss.str());
    }

    void insertPS(const Entity& e) {
        std::ostringstream oss;
        oss << "INSERT INTO " << EntityTraits<Entity>::tableName << " (";
        
        // Build column list (skip primary key if auto-increment)
        bool first = true;
        int paramIndex = 1;
        std::apply([&](auto&&... col) {
            ((buildColumnList(oss, col, first, true)), ...);
        }, EntityTraits<Entity>::columns);
        
        oss << ") VALUES (";
        
        // Build placeholder list
        first = true;
        std::apply([&](auto&&... col) {
            ((buildPlaceholderList(oss, col, first, paramIndex, true)), ...);
        }, EntityTraits<Entity>::columns);
        
        oss << ")";
        
        auto stmt = _conn.prepare(oss.str());
        
        // Bind parameters
        paramIndex = 1;
        std::apply([&](auto&&... col) {
            ((bindParameter(stmt.get(), col, e, paramIndex, true)), ...);
        }, EntityTraits<Entity>::columns);
        
        stmt->executeUpdate();
    }

    void insertBatch(const std::vector<Entity>& list) {
        auto txn = _conn.beginTransaction();
        try {
            for (const auto& entity : list) {
                insertPS(entity);
            }
            txn->commit();
        } catch (...) {
            txn->rollback();
            throw;
        }
    }

protected:
    void mapRowToEntity(IDBRow& row, Entity& e) {
        // Map columns to entity fields
        size_t colIndex = 0;
        std::apply([&](auto&&... col) {
            ((mapColumn(row, col, e, colIndex)), ...);
        }, EntityTraits<Entity>::columns);
    }

    template<typename Col>
    void mapColumn(IDBRow& row, const Col& col, Entity& e, size_t& colIndex) {
        if (colIndex >= row.columnCount()) {
            throw DBException("Column index out of range");
        }
        
        const auto& value = row[colIndex++];
        if (!value.isNull()) {
            using FieldType = typename std::remove_reference_t<decltype(e.*(col.member))>;
            
            if constexpr (std::is_same_v<FieldType, int>) {
                e.*(col.member) = value.asInt();
            } else if constexpr (std::is_same_v<FieldType, double>) {
                e.*(col.member) = value.asDouble();
            } else if constexpr (std::is_same_v<FieldType, std::string>) {
                e.*(col.member) = value.asString();
            }
        }
    }

    template<typename Col>
    void buildColumnList(std::ostringstream& oss, const Col& col, bool& first, bool skipPrimaryKey) {
        if (skipPrimaryKey && col.name == EntityTraits<Entity>::primaryKey) {
            return;  // Skip primary key for auto-increment
        }
        if (!first) oss << ", ";
        oss << col.name;
        first = false;
    }

    template<typename Col>
    void buildValuesList(std::ostringstream& oss, const Col& col, const Entity& e, bool& first, bool skipPrimaryKey) {
        if (skipPrimaryKey && col.name == EntityTraits<Entity>::primaryKey) {
            return;
        }
        if (!first) oss << ", ";
        
        using FieldType = typename std::remove_reference_t<decltype(e.*(col.member))>;
        if constexpr (std::is_same_v<FieldType, std::string>) {
            oss << "'" << escapeString(e.*(col.member)) << "'";
        } else {
            oss << e.*(col.member);
        }
        first = false;
    }

    template<typename Col>
    void buildUpdateSet(std::ostringstream& oss, const Col& col, const Entity& e, bool& first) {
        if (col.name == EntityTraits<Entity>::primaryKey) {
            return;  // Skip primary key in SET clause
        }
        if (!first) oss << ", ";
        
        oss << col.name << "=";
        using FieldType = typename std::remove_reference_t<decltype(e.*(col.member))>;
        if constexpr (std::is_same_v<FieldType, std::string>) {
            oss << "'" << escapeString(e.*(col.member)) << "'";
        } else {
            oss << e.*(col.member);
        }
        first = false;
    }

    template<typename Col>
    void buildWhereClause(std::ostringstream& oss, const Col& col, const Entity& e) {
        if (col.name == EntityTraits<Entity>::primaryKey) {
            using FieldType = typename std::remove_reference_t<decltype(e.*(col.member))>;
            if constexpr (std::is_same_v<FieldType, std::string>) {
                oss << "'" << escapeString(e.*(col.member)) << "'";
            } else {
                oss << e.*(col.member);
            }
        }
    }

    template<typename Col>
    void buildPlaceholderList(std::ostringstream& oss, const Col& col, bool& first, int& paramIndex, bool skipPrimaryKey) {
        if (skipPrimaryKey && col.name == EntityTraits<Entity>::primaryKey) {
            return;
        }
        if (!first) oss << ", ";
        oss << "$" << paramIndex++;
        first = false;
    }

    template<typename Col>
    void bindParameter(IDBPreparedStatement* stmt, const Col& col, const Entity& e, int& paramIndex, bool skipPrimaryKey) {
        if (skipPrimaryKey && col.name == EntityTraits<Entity>::primaryKey) {
            return;
        }
        
        using FieldType = typename std::remove_reference_t<decltype(e.*(col.member))>;
        if constexpr (std::is_same_v<FieldType, int>) {
            stmt->bindInt(paramIndex++, e.*(col.member));
        } else if constexpr (std::is_same_v<FieldType, double>) {
            stmt->bindDouble(paramIndex++, e.*(col.member));
        } else if constexpr (std::is_same_v<FieldType, std::string>) {
            stmt->bindString(paramIndex++, e.*(col.member));
        }
    }

    std::string escapeString(const std::string& str) {
        std::string escaped;
        for (char c : str) {
            if (c == '\'') {
                escaped += "''";  // SQL standard: double the single quote
            } else if (c == '\\') {
                escaped += "\\\\";  // Escape backslash for safety
            } else {
                escaped += c;
            }
        }
        return escaped;
    }

private:
    IDBConnection& _conn;
};
