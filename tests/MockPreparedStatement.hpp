#pragma once
#include "db/IDBPreparedStatement.hpp"
#include <string>
#include <vector>

class MockPreparedStatement : public IDBPreparedStatement {
public:
    MockPreparedStatement() = default;

    void bindInt(int index, int value) override;
    void bindDouble(int index, double value) override;
    void bindString(int index, const std::string& value) override;

    std::unique_ptr<IDBReader> executeQuery() override;
    void executeUpdate() override;

    std::size_t boundParamsCount() const { return _params.size(); }

private:
    std::vector<std::string> _params;
};
