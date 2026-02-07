# HFT Legacy Migration Framework

A modern C++ system designed to unify legacy database access with a clean, extensible architecture supporting:

- **Sybase DB-Lib** - Full support for Sybase databases
- **PostgreSQL libpq-fe** - Native PostgreSQL connectivity  
- **Compile-time reflection** (EntityTraits) - Type-safe entity mapping
- **Runtime schema Catalog** - Dynamic metadata management
- **Automatic entity & repository generation** - Code generation utilities
- **Prepared statements** - Secure, efficient SQL execution
- **Transactions** - ACID compliance with automatic rollback
- **Unit tests** - Comprehensive test coverage
- **Clean, extensible architecture** - SOLID principles throughout

## Features

### Database Abstraction Layer
- Unified interface for multiple database backends
- Connection pooling support
- Prepared statement API
- Transaction management
- Type-safe parameter binding

### ORM (Object-Relational Mapping)
- Generic repository pattern
- CRUD operations (Create, Read, Update, Delete)
- Compile-time type safety
- Automatic SQL generation

### Reflection System
- Compile-time entity traits
- Field metadata extraction
- Type information preservation
- Zero runtime overhead

### Schema Catalog
- Runtime table metadata
- SQL generation for DDL
- Multi-dialect support (PostgreSQL, Sybase)
- Schema introspection

### Code Generation
- Automatic entity class generation
- Repository class scaffolding
- Database-agnostic code

## Building

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15+
- PostgreSQL development libraries (optional)
- Sybase DB-Lib (optional)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/ChristophePichaud/HFT-LegacyMigrationFramework.git
cd HFT-LegacyMigrationFramework

# Create build directory
mkdir build && cd build

# Configure (PostgreSQL only)
cmake .. -DWITH_POSTGRESQL=ON -DWITH_SYBASE=OFF

# Or configure with both databases
cmake .. -DWITH_POSTGRESQL=ON -DWITH_SYBASE=ON

# Build
cmake --build .

# Run tests (requires running database server)
ctest
```

### Build Options
- `BUILD_TESTS` - Build unit tests (default: ON)
- `BUILD_EXAMPLES` - Build example programs (default: ON)
- `WITH_POSTGRESQL` - Enable PostgreSQL support (default: ON)
- `WITH_SYBASE` - Enable Sybase support (default: OFF)

## Quick Start

### Basic Database Connection

```cpp
#include "hft/db/PostgreSQLConnection.h"

int main() {
    auto conn = std::make_shared<hft::db::PostgreSQLConnection>();
    
    if (conn->open("host=localhost dbname=mydb user=user password=pass")) {
        conn->execute("SELECT version()");
        conn->close();
    }
    
    return 0;
}
```

### Using Prepared Statements

```cpp
auto stmt = conn->createStatement("SELECT * FROM users WHERE id = $1");
stmt->bindInt(1, 42);

auto result = stmt->executeQuery();
while (result->next()) {
    std::cout << result->getString(0) << std::endl;
}
```

### Transactions

```cpp
auto txn = conn->beginTransaction();

try {
    conn->execute("INSERT INTO users (name) VALUES ('John')");
    conn->execute("UPDATE accounts SET balance = balance - 100");
    txn->commit();
} catch (...) {
    txn->rollback();
}
```

### ORM Usage

```cpp
#include "hft/orm/Repository.h"
#include "hft/reflection/EntityTraits.h"

// Define entity
struct User {
    int64_t id;
    std::string name;
    std::string email;
};

// Define traits
namespace hft { namespace reflection {
template<>
struct EntityTraits<User> {
    static constexpr const char* tableName() { return "users"; }
    
    template<typename Func>
    static void forEachField(User& entity, Func&& func) {
        func("id", getFieldType<decltype(entity.id)>(), &entity.id, true, false);
        func("name", getFieldType<decltype(entity.name)>(), &entity.name, false, false);
        func("email", getFieldType<decltype(entity.email)>(), &entity.email, false, false);
    }
};
}}

// Use repository
int main() {
    auto conn = std::make_shared<hft::db::PostgreSQLConnection>();
    conn->open("...");
    
    hft::orm::Repository<User> userRepo(conn);
    
    // Insert
    User user{0, "Alice", "alice@example.com"};
    userRepo.insert(user);
    
    // Find
    auto found = userRepo.findById(1);
    if (found) {
        std::cout << found->name << std::endl;
    }
    
    // Update
    found->email = "newemail@example.com";
    userRepo.update(*found);
    
    // Delete
    userRepo.deleteById(1);
    
    return 0;
}
```

### Code Generation

```cpp
#include "hft/catalog/Catalog.h"
#include "hft/codegen/CodeGenerator.h"

int main() {
    using namespace hft::catalog;
    using namespace hft::codegen;
    
    // Define schema
    TableInfo table("products");
    table.addColumn(ColumnInfo("id", FieldType::INT64, true));
    table.addColumn(ColumnInfo("name", FieldType::STRING, false));
    table.addColumn(ColumnInfo("price", FieldType::DOUBLE, false));
    
    Catalog::instance().registerTable(table);
    
    // Generate entity class
    std::string entityCode = CodeGenerator::generateEntity(table);
    std::cout << entityCode << std::endl;
    
    // Generate repository
    std::string repoCode = CodeGenerator::generateRepository(table);
    std::cout << repoCode << std::endl;
    
    // Generate SQL
    std::string sql = Catalog::instance().generateCreateTableSQL("products", "postgresql");
    std::cout << sql << std::endl;
    
    return 0;
}
```

## Architecture

### Layer Overview

```
┌─────────────────────────────────────┐
│         Application Layer           │
│  (Your Business Logic & Entities)   │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│         ORM Layer                   │
│  - Repository<T>                    │
│  - EntityTraits<T>                  │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│    Database Abstraction Layer       │
│  - IConnection                      │
│  - IStatement                       │
│  - IResultSet                       │
│  - ITransaction                     │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│      Database Implementations       │
│  - PostgreSQLConnection             │
│  - SybaseConnection                 │
└─────────────────────────────────────┘
```

### Component Descriptions

#### Database Abstraction Layer (`hft/db/`)
- **IConnection**: Base interface for database connections
- **IStatement**: Prepared statement interface
- **IResultSet**: Query result iteration
- **ITransaction**: Transaction management

#### ORM Layer (`hft/orm/`)
- **Repository<T>**: Generic repository pattern implementation
- Provides CRUD operations for any entity type
- Automatic SQL generation based on entity traits

#### Reflection System (`hft/reflection/`)
- **EntityTraits<T>**: Compile-time reflection for entities
- Field metadata extraction
- Type mapping from C++ to SQL types

#### Catalog (`hft/catalog/`)
- **Catalog**: Runtime schema metadata registry
- **TableInfo**: Table and column metadata
- SQL DDL generation for multiple dialects

#### Code Generation (`hft/codegen/`)
- **CodeGenerator**: Automatic code generation
- Entity class generation from schema
- Repository class scaffolding

## Testing

The framework includes comprehensive unit tests using Google Test.

```bash
# Run all tests
cd build
ctest

# Run specific test suite
./tests/test_orm
./tests/test_catalog
./tests/test_postgresql  # Requires PostgreSQL server
./tests/test_sybase      # Requires Sybase server
```

### Test Coverage
- Database connection and disconnection
- Prepared statement execution
- Transaction commit and rollback
- ORM CRUD operations
- Catalog metadata management
- SQL generation for multiple dialects

## Examples

The `examples/` directory contains:
- `basic_usage.cpp` - Basic database operations
- `orm_usage.cpp` - Full ORM example
- `codegen_usage.cpp` - Code generation demonstration

Build and run examples:
```bash
cd build
./examples/example_basic
./examples/example_orm
./examples/example_codegen
```

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Authors

- Christophe Pichaud

## Acknowledgments

- PostgreSQL community for libpq-fe
- Sybase/SAP for DB-Lib
- Google Test framework
### *Legacy Migration Framework — C++ ORM, Catalog, DB Abstraction & Codegen*

Welcome to the **Legacy Migration Framework**, a modern C++ system designed to unify:

- Sybase DB‑Lib  
- PostgreSQL libpq‑fe  
- A compile‑time reflection system (EntityTraits)  
- A runtime schema Catalog  
- Automatic entity & repository generation  
- Prepared statements  
- Transactions  
- Unit tests  
- A clean, extensible architecture  
