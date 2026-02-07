#include "MockPreparedStatement.hpp"
#include "MockReader.hpp"

void MockPreparedStatement::bindInt(int index, int value) {
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void MockPreparedStatement::bindDouble(int index, double value) {
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void MockPreparedStatement::bindString(int index, const std::string& value) {
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = value;
}

std::unique_ptr<IDBReader> MockPreparedStatement::executeQuery() {
    return std::make_unique<MockReader>();
}

void MockPreparedStatement::executeUpdate() {
    // no-op
}
