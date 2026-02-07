#pragma once
#include "entity/BaseEntity.hpp"
#include "entity/EntityTraits.hpp"
#include "entity/Column.hpp"
#include <string>
#include <tuple>
#include <nlohmann/json.hpp>

class FXInstrument2 : public BaseEntity {
public:
    int _id{};
    int _userId{};
    int _instrumentId{};
    std::string _side;
    double _quantity{};
    double _price{};
    std::string _timestamp;

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["id"] = _id;
        j["userId"] = _userId;
        j["instrumentId"] = _instrumentId;
        j["side"] = _side;
        j["quantity"] = _quantity;
        j["price"] = _price;
        j["timestamp"] = _timestamp;
        return j;
    }
};

template<>
struct EntityTraits<FXInstrument2> {
    using Entity = FXInstrument2;

    static constexpr std::string_view tableName  = "FXInstrument2";
    static constexpr std::string_view primaryKey = "id";

    static constexpr auto columns = std::make_tuple(
        Column<Entity, int>{ "id", &Entity::_id },
        Column<Entity, int>{ "userId", &Entity::_userId },
        Column<Entity, int>{ "instrumentId", &Entity::_instrumentId },
        Column<Entity, std::string>{ "side", &Entity::_side },
        Column<Entity, double>{ "quantity", &Entity::_quantity },
        Column<Entity, double>{ "price", &Entity::_price },
        Column<Entity, std::string>{ "timestamp", &Entity::_timestamp }
    );
};
