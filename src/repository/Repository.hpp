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
        // Stub: you can implement literal SQL insert later
        (void)e;
        throw DBException("Repository::insert not implemented");
    }

    void update(const Entity& e) {
        (void)e;
        throw DBException("Repository::update not implemented");
    }

    void remove(const Entity& e) {
        (void)e;
        throw DBException("Repository::remove not implemented");
    }

    void insertPS(const Entity& e) {
        (void)e;
        throw DBException("Repository::insertPS not implemented");
    }

    void insertBatch(const std::vector<Entity>& list) {
        (void)list;
        throw DBException("Repository::insertBatch not implemented");
    }

protected:
    void mapRowToEntity(IDBRow& row, Entity& e) {
        // Very naive: assume columns in order of EntityTraits
        constexpr auto cols = EntityTraits<Entity>::columns;
        mapRowToEntityImpl(row, e, cols, std::make_index_sequence<std::tuple_size_v<decltype(cols)>>{});
    }

    template<std::size_t... I>
    void mapRowToEntityImpl(IDBRow& row, Entity& e,
                            const decltype(EntityTraits<Entity>::columns)& cols,
                            std::index_sequence<I...>) {
        // This is just a stub; real implementation would use type traits
        (void)row;
        (void)e;
        (void)cols;
        (void)sizeof...(I);
    }

private:
    IDBConnection& _conn;
};
