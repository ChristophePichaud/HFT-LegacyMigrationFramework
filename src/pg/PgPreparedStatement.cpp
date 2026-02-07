#include "PgPreparedStatement.hpp"
#include "db/DBException.hpp"
#include "pg/PgReader.hpp"

PgPreparedStatement::PgPreparedStatement(std::string sql)
    : _sql(std::move(sql)) {}

PgPreparedStatement::~PgPreparedStatement() = default;

void PgPreparedStatement::bindInt(int index, int value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindInt: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void PgPreparedStatement::bindDouble(int index, double value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindDouble: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void PgPreparedStatement::bindString(int index, const std::string& value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindString: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = value;
}

std::unique_ptr<IDBReader> PgPreparedStatement::executeQuery() {
    throw DBException("PgPreparedStatement::executeQuery not implemented");
}

void PgPreparedStatement::executeUpdate() {
    throw DBException("PgPreparedStatement::executeUpdate not implemented");
}
