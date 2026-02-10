#pragma once
#include "db/IDBConnection.hpp"
#include <memory>
#include <string>

// Forward declare libpq type
struct pg_conn;
typedef struct pg_conn PGconn;

class PgConnection : public IDBConnection {
public:
    explicit PgConnection(const std::string& conninfo);
    ~PgConnection() override;

    std::unique_ptr<IDBReader>
    executeQuery(const std::string& sql) override;

    std::unique_ptr<IDBPreparedStatement>
    prepare(const std::string& sql) override;

    std::unique_ptr<IDBTransaction>
    beginTransaction() override;

    // Access to underlying connection
    PGconn* getConnection() { return _conn; }

private:
    std::string _conninfo;
    PGconn* _conn{nullptr};
};
