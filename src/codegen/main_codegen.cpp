#include "catalog/Catalog.hpp"
#include "db/IDBConnection.hpp"
#include "db/IDBReader.hpp"
#include "db/IDBPreparedStatement.hpp"
#include "db/IDBTransaction.hpp"
#include "codegen/EntityGenerator.hpp"
#include "codegen/RepositoryGenerator.hpp"
#include "codegen/UnitTestGenerator.hpp"
#include <iostream>

class DummyConnection : public IDBConnection {
public:
    std::unique_ptr<IDBReader> executeQuery(const std::string&) override { return {}; }
    std::unique_ptr<IDBPreparedStatement> prepare(const std::string&) override { return {}; }
    std::unique_ptr<IDBTransaction> beginTransaction() override { return {}; }
};

int main() {
    DummyConnection conn;
    Catalog catalog(conn, DbDialect::PostgreSQL);

    EntityGenerator eg;
    RepositoryGenerator rg;
    UnitTestGenerator utg;

    eg.generateEntities(catalog, "src/entity/generated");
    eg.generateEntityTraits(catalog, "src/entity");
    rg.generateRepositories(catalog, "src/repository/generated");
    utg.generateTests(catalog, "tests");

    std::cout << "Code generation stub completed.\n";
    return 0;
}
