#include "SybPreparedStatement.hpp"
#include "db/DBException.hpp"
#include "sybase/SybReader.hpp"

SybPreparedStatement::SybPreparedStatement(std::string sql)
    : _sql(std::move(sql)) {}

SybPreparedStatement::~SybPreparedStatement() = default;

void SybPreparedStatement::bindInt(int index, int value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindInt: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void SybPreparedStatement::bindDouble(int index, double value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindDouble: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void SybPreparedStatement::bindString(int index, const std::string& value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindString: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = value;
}

std::unique_ptr<IDBReader> SybPreparedStatement::executeQuery() {
    throw DBException("SybPreparedStatement::executeQuery not implemented");
}

void SybPreparedStatement::executeUpdate() {
    throw DBException("SybPreparedStatement::executeUpdate not implemented");
}
