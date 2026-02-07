#pragma once
#include "db/IDBConnection.hpp"
#include <memory>
#include <string>

class SybConnection : public IDBConnection {
public:
    explicit SybConnection(const std::string& conninfo);
    ~SybConnection() override;

    std::unique_ptr<IDBReader>
    executeQuery(const std::string& sql) override;

    std::unique_ptr<IDBPreparedStatement>
    prepare(const std::string& sql) override;

    std::unique_ptr<IDBTransaction>
    beginTransaction() override;

private:
    std::string _conninfo;
};
