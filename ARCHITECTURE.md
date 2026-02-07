# Project Architecture and Design

## Overview

The HFT Legacy Migration Framework is designed with a layered architecture that promotes:
- **Separation of concerns**: Each layer has a specific responsibility
- **Abstraction**: Database-specific details are hidden behind interfaces
- **Extensibility**: New databases and features can be added easily
- **Type safety**: Compile-time reflection catches errors early
- **Performance**: Zero-overhead abstractions where possible

## Architecture Layers

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│  • Business Logic                                           │
│  • Entity Definitions                                       │
│  • Custom Repositories                                      │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    Code Generation Layer                    │
│  • CodeGenerator (generates entities & repos)               │
│  • Template-based code generation                           │
│  • Multi-dialect SQL DDL generation                         │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    ORM & Catalog Layer                      │
│  • Repository<T> (CRUD operations)                          │
│  • Catalog (runtime schema metadata)                        │
│  • EntityTraits<T> (compile-time reflection)               │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│               Database Abstraction Layer                    │
│  • IConnection (connection interface)                       │
│  • IStatement (prepared statement interface)                │
│  • IResultSet (result iteration interface)                  │
│  • ITransaction (transaction interface)                     │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│              Database Implementation Layer                   │
│  • PostgreSQLConnection (libpq-fe)                          │
│  • SybaseConnection (DB-Lib)                                │
│  • Future: MySQL, Oracle, etc.                              │
└─────────────────────────────────────────────────────────────┘
```

## Design Patterns

### 1. Repository Pattern

The Repository pattern provides a collection-like interface for accessing domain objects.

**Benefits:**
- Abstracts data access logic
- Centralizes data access code
- Makes testing easier (can mock repositories)
- Reduces code duplication

**Implementation:**
```cpp
template<typename T>
class Repository {
    std::optional<T> findById(int64_t id);
    std::vector<T> findAll();
    bool insert(T& entity);
    bool update(const T& entity);
    bool deleteById(int64_t id);
};
```

### 2. Interface Segregation

Each database operation has its own interface:
- `IConnection`: Connection management
- `IStatement`: SQL execution
- `IResultSet`: Result iteration
- `ITransaction`: Transaction control

This allows implementations to focus on one responsibility.

### 3. Dependency Injection

Repositories accept connections via constructor injection:

```cpp
Repository<User> userRepo(connection);
```

This makes testing easier and allows connection pooling.

### 4. Template Method Pattern

The ORM layer uses templates to provide generic implementations:

```cpp
template<typename T>
class Repository {
    // Generic CRUD operations work for any entity type T
    // that has EntityTraits<T> defined
};
```

### 5. Singleton Pattern

The Catalog uses a singleton to provide global access to schema metadata:

```cpp
Catalog::instance().registerTable(table);
```

### 6. RAII (Resource Acquisition Is Initialization)

Transactions use RAII for automatic cleanup:

```cpp
class Transaction {
    ~Transaction() {
        if (active_) rollback(); // Auto-rollback on exception
    }
};
```

## Key Components

### Database Abstraction Layer

**Purpose:** Provide a unified interface for multiple database backends.

**Files:**
- `include/hft/db/IConnection.h`
- `include/hft/db/IStatement.h`
- `include/hft/db/IResultSet.h`
- `include/hft/db/ITransaction.h`

**Design Decisions:**
- Pure virtual interfaces for maximum flexibility
- Prepared statements to prevent SQL injection
- Transaction support with ACID guarantees
- Type-safe parameter binding

### ORM Layer

**Purpose:** Map between C++ objects and database tables.

**Files:**
- `include/hft/orm/Repository.h`

**Features:**
- Automatic SQL generation from entity traits
- Type-safe CRUD operations
- Lazy loading support
- Extensible via inheritance

### Reflection System

**Purpose:** Provide compile-time type introspection.

**Files:**
- `include/hft/reflection/EntityTraits.h`

**Design:**
- Zero runtime overhead (all compile-time)
- Template specialization for each entity type
- Type mapping from C++ to SQL types
- Field iteration via callbacks

### Catalog

**Purpose:** Manage runtime schema metadata.

**Files:**
- `include/hft/catalog/Catalog.h`

**Capabilities:**
- Table and column metadata storage
- SQL DDL generation for multiple dialects
- Schema introspection
- Entity registration

### Code Generation

**Purpose:** Generate boilerplate code from schema.

**Files:**
- `include/hft/codegen/CodeGenerator.h`

**Generates:**
- Entity classes from table metadata
- Repository classes for entities
- SQL DDL statements
- EntityTraits specializations

## Data Flow

### Query Execution Flow

```
Application
    ↓ calls findById()
Repository<T>
    ↓ builds SQL
    ↓ creates statement
IConnection
    ↓ prepares statement
PostgreSQLConnection
    ↓ calls PQprepare()
PostgreSQL Server
    ↓ returns PGresult
PostgreSQLResultSet
    ↓ maps to entity
Repository<T>
    ↓ returns std::optional<T>
Application
```

### Insert Operation Flow

```
Application
    ↓ entity data
Repository<T>
    ↓ uses EntityTraits to iterate fields
    ↓ generates INSERT SQL
    ↓ binds parameters
IStatement
    ↓ executeUpdate()
PostgreSQLStatement
    ↓ PQexecPrepared()
PostgreSQL Server
    ↓ returns affected rows
Repository<T>
    ↓ returns success/failure
Application
```

### Transaction Flow

```
Application
    ↓ beginTransaction()
IConnection
    ↓ creates Transaction
PostgreSQLTransaction
    ↓ executes "BEGIN"
PostgreSQL Server
    ↓ transaction started
Application
    ↓ performs operations
    ↓ commit() or rollback()
PostgreSQLTransaction
    ↓ executes "COMMIT" or "ROLLBACK"
PostgreSQL Server
    ↓ transaction completed
Application
```

## Type System

### C++ to SQL Type Mapping

| C++ Type      | PostgreSQL   | Sybase       | FieldType      |
|---------------|--------------|--------------|----------------|
| int32_t       | INTEGER      | INT          | INT32          |
| int64_t       | BIGINT       | BIGINT       | INT64          |
| double        | DOUBLE       | FLOAT        | DOUBLE         |
| std::string   | TEXT         | VARCHAR(255) | STRING         |
| bool          | BOOLEAN      | BIT          | BOOL           |

### Extensibility

New types can be added by:
1. Adding to `FieldType` enum
2. Updating `getFieldType<T>()` template
3. Adding SQL mappings for each dialect
4. Implementing bind/get methods in adapters

## Error Handling Strategy

### Principles

1. **No Exceptions**: Core library doesn't throw exceptions
2. **Return Values**: Use `std::optional` or `nullptr` for failures
3. **Error Messages**: Provide `getLastError()` methods
4. **Logging**: Log errors for debugging
5. **Fail Safe**: Prefer safety over performance

### Example

```cpp
std::optional<User> findById(int64_t id) {
    auto stmt = connection_->createStatement(sql);
    if (!stmt) {
        // Log: "Failed to create statement"
        return std::nullopt;
    }
    
    auto result = stmt->executeQuery();
    if (!result) {
        // Log: "Query failed: " + connection_->getLastError()
        return std::nullopt;
    }
    
    if (!result->next()) {
        // Not found, normal case
        return std::nullopt;
    }
    
    return mapResultToEntity(result);
}
```

## Performance Considerations

### Prepared Statements

- Reusable statements avoid re-parsing SQL
- Parameter binding is efficient
- Reduced SQL injection risk

### Connection Pooling

- Reuse connections to reduce overhead
- Configurable pool size
- Thread-safe access

### Batch Operations

- Process multiple entities in one transaction
- Reduces network round-trips
- Better throughput for bulk operations

### Lazy Loading

- Load related entities only when needed
- Reduces initial query overhead
- Configurable eager loading for known access patterns

## Security

### SQL Injection Prevention

- **Always use prepared statements**
- Never concatenate user input into SQL
- Parameterized queries throughout

### Connection Security

- Support for SSL/TLS connections
- Encrypted credentials
- Connection string validation

### Access Control

- Repository-level permissions
- Row-level security (future)
- Audit logging (future)

## Testing Strategy

### Unit Tests

- Test each component in isolation
- Use mocks for dependencies
- Focus on logic correctness

### Integration Tests

- Test with real databases
- Verify SQL generation
- Check type conversions

### Performance Tests

- Benchmark query execution
- Measure connection overhead
- Profile memory usage

## Future Enhancements

### Planned Features

1. **Connection Pooling**: Built-in pool management
2. **Caching Layer**: Query result caching
3. **Migrations**: Schema version management
4. **Query Builder**: Fluent query API
5. **Relationships**: One-to-many, many-to-many support
6. **Validation**: Entity validation before persistence
7. **Events**: Pre/post operation hooks
8. **Logging**: Structured query logging
9. **Metrics**: Performance metrics collection
10. **Admin UI**: Web-based schema browser

### Database Support Roadmap

- [x] PostgreSQL
- [x] Sybase
- [ ] MySQL/MariaDB
- [ ] Oracle
- [ ] SQL Server
- [ ] SQLite
- [ ] MongoDB (document store)

## Conclusion

The HFT Legacy Migration Framework provides a solid foundation for database access in C++ applications. Its layered architecture, clear abstractions, and extensible design make it suitable for both legacy migration projects and new development.

The framework balances:
- **Simplicity**: Easy to use for common cases
- **Power**: Advanced features available when needed
- **Safety**: Type-safe and SQL injection resistant
- **Performance**: Efficient implementations with minimal overhead
- **Portability**: Works across multiple databases and platforms
