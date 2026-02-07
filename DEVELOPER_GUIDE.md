# Developer Guide

## Extending the Framework

This guide explains how to extend the HFT Legacy Migration Framework with new features and database adapters.

## Adding a New Database Adapter

To add support for a new database (e.g., MySQL, Oracle, SQL Server):

### 1. Create Connection Interface Implementation

Create header and implementation files:
- `include/hft/db/MyDatabaseConnection.h`
- `src/db/MyDatabaseConnection.cpp`

```cpp
#pragma once

#include "IConnection.h"
#include "IStatement.h"
#include "IResultSet.h"
#include "ITransaction.h"
// Include your database library headers

namespace hft {
namespace db {

class MyDatabaseConnection : public IConnection {
public:
    MyDatabaseConnection();
    ~MyDatabaseConnection() override;

    bool open(const std::string& connectionString) override;
    void close() override;
    bool isOpen() const override;
    std::shared_ptr<IStatement> createStatement(const std::string& sql) override;
    std::shared_ptr<ITransaction> beginTransaction() override;
    bool execute(const std::string& sql) override;
    std::string getLastError() const override;

private:
    // Database-specific connection handle
    void* dbHandle_;
    std::string lastError_;
};

} // namespace db
} // namespace hft
```

### 2. Implement Statement, ResultSet, and Transaction

Follow the same pattern as PostgreSQL and Sybase implementations.

### 3. Update CMakeLists.txt

Add build options and library detection:

```cmake
option(WITH_MYDATABASE "Build with MyDatabase support" OFF)

if(WITH_MYDATABASE)
    find_library(MYDATABASE_LIBRARY NAMES mydb)
    if(MYDATABASE_LIBRARY)
        include_directories(${MYDATABASE_INCLUDE_DIR})
        add_definitions(-DWITH_MYDATABASE)
        list(APPEND DB_SOURCES src/db/MyDatabaseConnection.cpp)
        target_link_libraries(hft-legacy-migration ${MYDATABASE_LIBRARY})
    endif()
endif()
```

### 4. Add to Main Header

Update `include/hft/hft.h`:

```cpp
#ifdef WITH_MYDATABASE
#include "hft/db/MyDatabaseConnection.h"
#endif
```

## Creating Custom Entity Types

### Define Your Entity

```cpp
struct Customer {
    int64_t id = 0;
    std::string name = "";
    std::string email = "";
    std::string phone = "";
    bool active = true;
};
```

### Define Entity Traits

```cpp
namespace hft { namespace reflection {

template<>
struct EntityTraits<Customer> {
    static constexpr const char* tableName() { return "customers"; }
    static constexpr size_t fieldCount() { return 5; }
    
    template<typename Func>
    static void forEachField(Customer& entity, Func&& func) {
        func("id", getFieldType<decltype(entity.id)>(), &entity.id, true, false);
        func("name", getFieldType<decltype(entity.name)>(), &entity.name, false, false);
        func("email", getFieldType<decltype(entity.email)>(), &entity.email, false, false);
        func("phone", getFieldType<decltype(entity.phone)>(), &entity.phone, false, true);
        func("active", getFieldType<decltype(entity.active)>(), &entity.active, false, false);
    }
};

}} // namespace hft::reflection
```

### Use the Repository

```cpp
auto conn = std::make_shared<hft::db::PostgreSQLConnection>();
conn->open("...");

hft::orm::Repository<Customer> customerRepo(conn);

// Insert
Customer customer;
customer.name = "Alice";
customer.email = "alice@example.com";
customerRepo.insert(customer);

// Find
auto found = customerRepo.findById(1);
```

## Extending the Repository Pattern

### Create a Custom Repository

```cpp
class CustomerRepository : public hft::orm::Repository<Customer> {
public:
    using hft::orm::Repository<Customer>::Repository;
    
    // Custom query: find by email
    std::optional<Customer> findByEmail(const std::string& email) {
        auto stmt = connection_->createStatement(
            "SELECT * FROM customers WHERE email = $1"
        );
        stmt->bindString(1, email);
        
        auto result = stmt->executeQuery();
        if (!result || !result->next()) {
            return std::nullopt;
        }
        
        return mapResultToEntity(result);
    }
    
    // Custom query: find active customers
    std::vector<Customer> findActiveCustomers() {
        auto stmt = connection_->createStatement(
            "SELECT * FROM customers WHERE active = true"
        );
        
        auto result = stmt->executeQuery();
        std::vector<Customer> customers;
        
        if (result) {
            while (result->next()) {
                customers.push_back(mapResultToEntity(result).value());
            }
        }
        
        return customers;
    }
};
```

## Adding New Field Types

To support custom field types (e.g., Date, UUID):

### 1. Add to FieldType Enum

In `include/hft/reflection/EntityTraits.h`:

```cpp
enum class FieldType {
    INT32,
    INT64,
    DOUBLE,
    STRING,
    BOOL,
    DATE,      // Add new type
    UUID       // Add new type
};
```

### 2. Update getFieldType Template

```cpp
template<typename T>
constexpr FieldType getFieldType() {
    // ... existing code ...
    else if constexpr (std::is_same_v<T, Date>) {
        return FieldType::DATE;
    } else if constexpr (std::is_same_v<T, UUID>) {
        return FieldType::UUID;
    }
    // ...
}
```

### 3. Update SQL Type Mapping

In `include/hft/catalog/Catalog.h`:

```cpp
std::string getTypeSQL(reflection::FieldType type, const std::string& dialect) const {
    if (dialect == "postgresql") {
        switch (type) {
            // ... existing cases ...
            case reflection::FieldType::DATE: return "DATE";
            case reflection::FieldType::UUID: return "UUID";
        }
    }
    // ...
}
```

### 4. Update Statement Binding

Add new bind methods to `IStatement`:

```cpp
virtual void bindDate(int index, const Date& value) = 0;
virtual void bindUUID(int index, const UUID& value) = 0;
```

Implement in each database adapter.

## Code Generation Best Practices

### Generate Multiple Files

```cpp
void generateEntityFiles(const std::string& outputDir) {
    auto& catalog = hft::catalog::Catalog::instance();
    
    for (const auto& tableName : catalog.getTableNames()) {
        const auto* table = catalog.getTable(tableName);
        
        // Generate header
        std::string headerPath = outputDir + "/entities/" + tableName + ".h";
        std::string headerCode = CodeGenerator::generateEntity(*table);
        writeFile(headerPath, headerCode);
        
        // Generate repository
        std::string repoPath = outputDir + "/repositories/" + tableName + "_repository.h";
        std::string repoCode = CodeGenerator::generateRepository(*table);
        writeFile(repoPath, repoCode);
    }
}
```

### Custom Code Templates

Extend `CodeGenerator` with your own templates:

```cpp
class MyCodeGenerator : public hft::codegen::CodeGenerator {
public:
    static std::string generateServiceLayer(const catalog::TableInfo& table) {
        std::ostringstream out;
        
        out << "#pragma once\n\n";
        out << "#include \"" << table.name << "_repository.h\"\n\n";
        
        std::string className = toPascalCase(table.name);
        
        out << "class " << className << "Service {\n";
        out << "public:\n";
        out << "    " << className << "Service(std::shared_ptr<" << className << "Repository> repo)\n";
        out << "        : repository_(repo) {}\n\n";
        out << "    // Add business logic methods here\n\n";
        out << "private:\n";
        out << "    std::shared_ptr<" << className << "Repository> repository_;\n";
        out << "};\n";
        
        return out.str();
    }
};
```

## Transaction Patterns

### Automatic Rollback with RAII

```cpp
class TransactionGuard {
public:
    TransactionGuard(std::shared_ptr<hft::db::IConnection> conn)
        : transaction_(conn->beginTransaction()) {}
    
    ~TransactionGuard() {
        if (transaction_ && transaction_->isActive()) {
            transaction_->rollback();
        }
    }
    
    void commit() {
        if (transaction_) {
            transaction_->commit();
        }
    }

private:
    std::shared_ptr<hft::db::ITransaction> transaction_;
};

// Usage
void transferFunds(Repository<Account>& repo, int64_t from, int64_t to, double amount) {
    TransactionGuard txn(repo.connection());
    
    auto fromAccount = repo.findById(from).value();
    auto toAccount = repo.findById(to).value();
    
    fromAccount.balance -= amount;
    toAccount.balance += amount;
    
    repo.update(fromAccount);
    repo.update(toAccount);
    
    txn.commit(); // Only commits if we reach here
}
```

## Performance Optimization

### Batch Operations

```cpp
template<typename T>
class BatchRepository : public Repository<T> {
public:
    using Repository<T>::Repository;
    
    bool batchInsert(const std::vector<T>& entities) {
        auto txn = this->connection_->beginTransaction();
        
        try {
            for (const auto& entity : entities) {
                this->insert(const_cast<T&>(entity));
            }
            return txn->commit();
        } catch (...) {
            txn->rollback();
            return false;
        }
    }
};
```

### Connection Pooling

```cpp
class ConnectionPool {
public:
    static ConnectionPool& instance() {
        static ConnectionPool pool;
        return pool;
    }
    
    std::shared_ptr<hft::db::IConnection> acquire(const std::string& connStr) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!freeConnections_.empty()) {
            auto conn = freeConnections_.back();
            freeConnections_.pop_back();
            return conn;
        }
        
        auto conn = std::make_shared<hft::db::PostgreSQLConnection>();
        conn->open(connStr);
        return conn;
    }
    
    void release(std::shared_ptr<hft::db::IConnection> conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        freeConnections_.push_back(conn);
    }

private:
    std::vector<std::shared_ptr<hft::db::IConnection>> freeConnections_;
    std::mutex mutex_;
};
```

## Testing Your Extensions

### Create Mock Implementations

```cpp
class MockConnection : public hft::db::IConnection {
    // Implement all methods for testing
};

TEST(MyTest, CustomQueryWorks) {
    auto mockConn = std::make_shared<MockConnection>();
    CustomerRepository repo(mockConn);
    
    auto result = repo.findByEmail("test@example.com");
    EXPECT_TRUE(result.has_value());
}
```

### Integration Tests

```cpp
TEST(IntegrationTest, EndToEndWorkflow) {
    // Requires running database
    auto conn = std::make_shared<hft::db::PostgreSQLConnection>();
    ASSERT_TRUE(conn->open("..."));
    
    // Test full workflow
    CustomerRepository repo(conn);
    
    Customer customer;
    customer.name = "Test";
    customer.email = "test@test.com";
    
    ASSERT_TRUE(repo.insert(customer));
    auto found = repo.findByEmail("test@test.com");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Test");
    
    conn->close();
}
```

## Contributing

When contributing to the framework:

1. Follow the existing code style
2. Add unit tests for new features
3. Update documentation
4. Ensure CMake builds work with new dependencies
5. Test with multiple database backends if applicable

## Best Practices

1. **Always use prepared statements** to prevent SQL injection
2. **Use transactions** for multi-step operations
3. **Handle errors gracefully** and provide meaningful error messages
4. **Keep entity classes simple** - just data, no logic
5. **Use repositories for data access** - isolate database logic
6. **Leverage compile-time reflection** - catch errors at compile time
7. **Generate code when possible** - reduce boilerplate
8. **Test thoroughly** - unit tests, integration tests, and real database tests
