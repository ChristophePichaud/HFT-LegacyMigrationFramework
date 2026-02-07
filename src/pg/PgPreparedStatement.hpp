#pragma once
#include "db/IDBPreparedStatement.hpp"
#include <string>
#include <vector>

class PgPreparedStatement : public IDBPreparedStatement {
public:
    explicit PgPreparedStatement(std::string sql);
    ~PgPreparedStatement() override;

    void bindInt(int index, int value) override;
    void bindDouble(int index, double value) override;
    void bindString(int index, const std::string& value) override;

    std::unique_ptr<IDBReader> executeQuery() override;
    void executeUpdate() override;

private:
    std::string _sql;
    std::vector<std::string> _params;
};
