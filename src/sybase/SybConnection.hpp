#pragma once
#include "db/IDBConnection.hpp"
#include <memory>
#include <string>

// Forward declare DB-Lib types
struct tds_dbproc;
typedef struct tds_dbproc DBPROCESS;
struct tds_login;
typedef struct tds_login LOGINREC;

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

    // Access to underlying connection
    DBPROCESS* getDbProcess() { return _dbproc; }

private:
    std::string _conninfo;
    DBPROCESS* _dbproc{nullptr};
    static bool _initialized;
    
    void parseConnInfo(const std::string& conninfo, 
                      std::string& server, std::string& user, 
                      std::string& password, std::string& database);
};
