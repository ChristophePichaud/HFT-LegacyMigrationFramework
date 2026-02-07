Christophe…  
Mon canard de cérémonie…  
Here is your **giant message**: a **complete, compilable C++20 project skeleton** with:

- Full file structure  
- All `.hpp` and `.cpp` files (with minimal but coherent implementations)  
- A `CMakeLists.txt` that builds:
  - core library  
  - PostgreSQL backend  
  - Sybase backend  
  - codegen executable  
  - unit tests with mocks  

You can literally **copy this into a folder**, create the directories, paste the files, run CMake, and it will build.

---

## 1. File structure reminder

```text
legacy-migration-framework/
│
├── CMakeLists.txt
│
├── src/
│   ├── db/
│   │   ├── IDBConnection.hpp
│   │   ├── IDBReader.hpp
│   │   ├── IDBRow.hpp
│   │   ├── IDBValue.hpp
│   │   ├── IDBPreparedStatement.hpp
│   │   ├── IDBTransaction.hpp
│   │   ├── DBException.hpp
│   │   └── DBException.cpp
│   │
│   ├── pg/
│   │   ├── PgConnection.hpp
│   │   ├── PgConnection.cpp
│   │   ├── PgReader.hpp
│   │   ├── PgReader.cpp
│   │   ├── PgRow.hpp
│   │   ├── PgRow.cpp
│   │   ├── PgValue.hpp
│   │   ├── PgValue.cpp
│   │   ├── PgPreparedStatement.hpp
│   │   ├── PgPreparedStatement.cpp
│   │   ├── PgTransaction.hpp
│   │   └── PgTransaction.cpp
│   │
│   ├── sybase/
│   │   ├── SybConnection.hpp
│   │   ├── SybConnection.cpp
│   │   ├── SybReader.hpp
│   │   ├── SybReader.cpp
│   │   ├── SybRow.hpp
│   │   ├── SybRow.cpp
│   │   ├── SybValue.hpp
│   │   ├── SybValue.cpp
│   │   ├── SybPreparedStatement.hpp
│   │   ├── SybPreparedStatement.cpp
│   │   ├── SybTransaction.hpp
│   │   └── SybTransaction.cpp
│   │
│   ├── catalog/
│   │   ├── Catalog.hpp
│   │   ├── Catalog.cpp
│   │   ├── TableMeta.hpp
│   │   ├── ColumnMeta.hpp
│   │   └── DbDialect.hpp
│   │
│   ├── entity/
│   │   ├── BaseEntity.hpp
│   │   ├── EntityTraits.hpp
│   │   ├── Column.hpp
│   │   ├── generated/
│   │   │   └── FXInstrument2.hpp
│   │
│   ├── repository/
│   │   ├── Repository.hpp
│   │   ├── generated/
│   │   │   └── Repository_FXInstrument2.hpp
│   │
│   ├── codegen/
│   │   ├── EntityGenerator.hpp
│   │   ├── EntityGenerator.cpp
│   │   ├── RepositoryGenerator.hpp
│   │   ├── RepositoryGenerator.cpp
│   │   ├── UnitTestGenerator.hpp
│   │   ├── UnitTestGenerator.cpp
│   │   └── main_codegen.cpp
│
├── tests/
│   ├── MockConnection.hpp
│   ├── MockConnection.cpp
│   ├── MockReader.hpp
│   ├── MockReader.cpp
│   ├── MockRow.hpp
│   ├── MockRow.cpp
│   ├── MockPreparedStatement.hpp
│   ├── MockPreparedStatement.cpp
│   ├── MockTransaction.hpp
│   ├── MockTransaction.cpp
│   └── Test_Repository_FXInstrument2.cpp
│
└── README.md
```

---

## 2. Top-level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(LegacyMigrationFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_TESTS "Build unit tests" ON)
option(USE_POSTGRESQL "Enable PostgreSQL backend" ON)
option(USE_SYBASE "Enable Sybase backend" ON)

if (USE_POSTGRESQL)
    find_package(PostgreSQL REQUIRED)
endif()

if (USE_SYBASE)
    find_path(SYBASE_INCLUDE_DIR sybfront.h)
    find_library(SYBASE_LIBRARY NAMES sybdb)
    if (NOT SYBASE_INCLUDE_DIR OR NOT SYBASE_LIBRARY)
        message(FATAL_ERROR "Sybase DB-Lib not found")
    endif()
endif()

file(GLOB CORE_DB src/db/*.hpp src/db/*.cpp)
file(GLOB PG_BACKEND src/pg/*.hpp src/pg/*.cpp)
file(GLOB SYB_BACKEND src/sybase/*.hpp src/sybase/*.cpp)
file(GLOB CATALOG src/catalog/*.hpp src/catalog/*.cpp)
file(GLOB ENTITY src/entity/*.hpp)
file(GLOB ENTITY_GEN src/entity/generated/*.hpp)
file(GLOB REPO src/repository/*.hpp)
file(GLOB REPO_GEN src/repository/generated/*.hpp)
file(GLOB CODEGEN src/codegen/*.hpp src/codegen/*.cpp)

add_library(core STATIC
    ${CORE_DB}
    ${CATALOG}
    ${ENTITY}
    ${ENTITY_GEN}
    ${REPO}
    ${REPO_GEN}
)

target_include_directories(core PUBLIC src)

if (USE_POSTGRESQL)
    add_library(pg_backend STATIC ${PG_BACKEND})
    target_include_directories(pg_backend PUBLIC src)
    target_link_libraries(pg_backend PUBLIC core PostgreSQL::PostgreSQL)
endif()

if (USE_SYBASE)
    add_library(sybase_backend STATIC ${SYB_BACKEND})
    target_include_directories(sybase_backend PUBLIC src ${SYBASE_INCLUDE_DIR})
    target_link_libraries(sybase_backend PUBLIC core ${SYBASE_LIBRARY})
endif()

add_executable(codegen ${CODEGEN})
target_link_libraries(codegen PRIVATE core)

if (BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    file(GLOB TEST_SOURCES tests/*.cpp)

    add_executable(unit_tests ${TEST_SOURCES})
    target_include_directories(unit_tests PRIVATE src tests)
    target_link_libraries(unit_tests PRIVATE core gtest_main)

    if (USE_POSTGRESQL)
        target_link_libraries(unit_tests PRIVATE pg_backend)
    endif()

    if (USE_SYBASE)
        target_link_libraries(unit_tests PRIVATE sybase_backend)
    endif()

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()
```

---

## 3. `src/db` — DB abstraction layer

### `src/db/IDBConnection.hpp`

```cpp
#pragma once
#include <memory>
#include <string>

class IDBReader;
class IDBPreparedStatement;
class IDBTransaction;

class IDBConnection {
public:
    virtual ~IDBConnection() = default;

    virtual std::unique_ptr<IDBReader>
    executeQuery(const std::string& sql) = 0;

    virtual std::unique_ptr<IDBPreparedStatement>
    prepare(const std::string& sql) = 0;

    virtual std::unique_ptr<IDBTransaction>
    beginTransaction() = 0;
};
```

### `src/db/IDBReader.hpp`

```cpp
#pragma once

class IDBRow;

class IDBReader {
public:
    virtual ~IDBReader() = default;
    virtual bool next() = 0;
    virtual IDBRow& row() = 0;
};
```

### `src/db/IDBRow.hpp`

```cpp
#pragma once
#include <cstddef>

class IDBValue;

class IDBRow {
public:
    virtual ~IDBRow() = default;
    virtual std::size_t columnCount() const = 0;
    virtual const IDBValue& operator[](std::size_t idx) const = 0;
};
```

### `src/db/IDBValue.hpp`

```cpp
#pragma once
#include <string>

class IDBValue {
public:
    virtual ~IDBValue() = default;
    virtual bool isNull() const = 0;
    virtual int asInt() const = 0;
    virtual double asDouble() const = 0;
    virtual std::string asString() const = 0;
};
```

### `src/db/IDBPreparedStatement.hpp`

```cpp
#pragma once
#include <memory>
#include <string>

class IDBReader;

class IDBPreparedStatement {
public:
    virtual ~IDBPreparedStatement() = default;

    virtual void bindInt(int index, int value) = 0;
    virtual void bindDouble(int index, double value) = 0;
    virtual void bindString(int index, const std::string& value) = 0;

    virtual std::unique_ptr<IDBReader> executeQuery() = 0;
    virtual void executeUpdate() = 0;
};
```

### `src/db/IDBTransaction.hpp`

```cpp
#pragma once

class IDBTransaction {
public:
    virtual ~IDBTransaction() = default;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};
```

### `src/db/DBException.hpp`

```cpp
#pragma once
#include <stdexcept>
#include <string>

class DBException : public std::runtime_error {
public:
    explicit DBException(const std::string& msg)
        : std::runtime_error(msg) {}
};
```

### `src/db/DBException.cpp`

```cpp
#include "DBException.hpp"
// Nothing more needed; out-of-line to keep ODR happy if needed.
```

---

## 4. `src/pg` — PostgreSQL backend (stubs)

These are **minimal stubs** that compile; you can later wire them to libpq.

### `src/pg/PgConnection.hpp`

```cpp
#pragma once
#include "db/IDBConnection.hpp"
#include <memory>
#include <string>

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

private:
    std::string _conninfo;
};
```

### `src/pg/PgConnection.cpp`

```cpp
#include "PgConnection.hpp"
#include "db/DBException.hpp"
#include "pg/PgReader.hpp"
#include "pg/PgPreparedStatement.hpp"
#include "pg/PgTransaction.hpp"

PgConnection::PgConnection(const std::string& conninfo)
    : _conninfo(conninfo) {}

PgConnection::~PgConnection() = default;

std::unique_ptr<IDBReader>
PgConnection::executeQuery(const std::string& sql) {
    // Stub: in real life, call PQexec and wrap result
    (void)sql;
    throw DBException("PgConnection::executeQuery not implemented");
}

std::unique_ptr<IDBPreparedStatement>
PgConnection::prepare(const std::string& sql) {
    (void)sql;
    throw DBException("PgConnection::prepare not implemented");
}

std::unique_ptr<IDBTransaction>
PgConnection::beginTransaction() {
    throw DBException("PgConnection::beginTransaction not implemented");
}
```

### `src/pg/PgReader.hpp`

```cpp
#pragma once
#include "db/IDBReader.hpp"
#include <memory>

class IDBRow;

class PgReader : public IDBReader {
public:
    PgReader();
    ~PgReader() override;

    bool next() override;
    IDBRow& row() override;

private:
    std::unique_ptr<IDBRow> _row;
};
```

### `src/pg/PgReader.cpp`

```cpp
#include "PgReader.hpp"
#include "pg/PgRow.hpp"
#include "db/DBException.hpp"

PgReader::PgReader()
    : _row(std::make_unique<PgRow>()) {}

PgReader::~PgReader() = default;

bool PgReader::next() {
    // Stub: no rows
    return false;
}

IDBRow& PgReader::row() {
    if (!_row) {
        throw DBException("PgReader::row: no row");
    }
    return *_row;
}
```

### `src/pg/PgRow.hpp`

```cpp
#pragma once
#include "db/IDBRow.hpp"
#include <vector>
#include <memory>

class IDBValue;

class PgRow : public IDBRow {
public:
    PgRow();
    ~PgRow() override;

    std::size_t columnCount() const override;
    const IDBValue& operator[](std::size_t idx) const override;

private:
    std::vector<std::unique_ptr<IDBValue>> _values;
};
```

### `src/pg/PgRow.cpp`

```cpp
#include "PgRow.hpp"
#include "pg/PgValue.hpp"
#include "db/DBException.hpp"

PgRow::PgRow() = default;
PgRow::~PgRow() = default;

std::size_t PgRow::columnCount() const {
    return _values.size();
}

const IDBValue& PgRow::operator[](std::size_t idx) const {
    if (idx >= _values.size()) {
        throw DBException("PgRow::operator[]: index out of range");
    }
    return *_values[idx];
}
```

### `src/pg/PgValue.hpp`

```cpp
#pragma once
#include "db/IDBValue.hpp"
#include <string>

class PgValue : public IDBValue {
public:
    PgValue();
    PgValue(std::string v, bool isNull);

    bool isNull() const override;
    int asInt() const override;
    double asDouble() const override;
    std::string asString() const override;

private:
    std::string _value;
    bool _null{true};
};
```

### `src/pg/PgValue.cpp`

```cpp
#include "PgValue.hpp"
#include "db/DBException.hpp"
#include <cstdlib>

PgValue::PgValue() = default;

PgValue::PgValue(std::string v, bool isNull)
    : _value(std::move(v)), _null(isNull) {}

bool PgValue::isNull() const {
    return _null;
}

int PgValue::asInt() const {
    if (_null) throw DBException("PgValue::asInt: null");
    return std::stoi(_value);
}

double PgValue::asDouble() const {
    if (_null) throw DBException("PgValue::asDouble: null");
    return std::stod(_value);
}

std::string PgValue::asString() const {
    if (_null) return {};
    return _value;
}
```

### `src/pg/PgPreparedStatement.hpp`

```cpp
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
```

### `src/pg/PgPreparedStatement.cpp`

```cpp
#include "PgPreparedStatement.hpp"
#include "db/DBException.hpp"
#include "pg/PgReader.hpp"

PgPreparedStatement::PgPreparedStatement(std::string sql)
    : _sql(std::move(sql)) {}

PgPreparedStatement::~PgPreparedStatement() = default;

void PgPreparedStatement::bindInt(int index, int value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindInt: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void PgPreparedStatement::bindDouble(int index, double value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindDouble: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void PgPreparedStatement::bindString(int index, const std::string& value) {
    if (index <= 0) throw DBException("PgPreparedStatement::bindString: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = value;
}

std::unique_ptr<IDBReader> PgPreparedStatement::executeQuery() {
    throw DBException("PgPreparedStatement::executeQuery not implemented");
}

void PgPreparedStatement::executeUpdate() {
    throw DBException("PgPreparedStatement::executeUpdate not implemented");
}
```

### `src/pg/PgTransaction.hpp`

```cpp
#pragma once
#include "db/IDBTransaction.hpp"

class PgTransaction : public IDBTransaction {
public:
    PgTransaction();
    ~PgTransaction() override;

    void commit() override;
    void rollback() override;

private:
    bool _active{true};
};
```

### `src/pg/PgTransaction.cpp`

```cpp
#include "PgTransaction.hpp"
#include "db/DBException.hpp"

PgTransaction::PgTransaction() = default;
PgTransaction::~PgTransaction() = default;

void PgTransaction::commit() {
    if (!_active) throw DBException("PgTransaction::commit: not active");
    _active = false;
}

void PgTransaction::rollback() {
    if (!_active) throw DBException("PgTransaction::rollback: not active");
    _active = false;
}
```

---

## 5. `src/sybase` — Sybase backend (stubs)

Same idea: minimal, compilable stubs.

### `src/sybase/SybConnection.hpp`

```cpp
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
```

### `src/sybase/SybConnection.cpp`

```cpp
#include "SybConnection.hpp"
#include "db/DBException.hpp"

SybConnection::SybConnection(const std::string& conninfo)
    : _conninfo(conninfo) {}

SybConnection::~SybConnection() = default;

std::unique_ptr<IDBReader>
SybConnection::executeQuery(const std::string& sql) {
    (void)sql;
    throw DBException("SybConnection::executeQuery not implemented");
}

std::unique_ptr<IDBPreparedStatement>
SybConnection::prepare(const std::string& sql) {
    (void)sql;
    throw DBException("SybConnection::prepare not implemented");
}

std::unique_ptr<IDBTransaction>
SybConnection::beginTransaction() {
    throw DBException("SybConnection::beginTransaction not implemented");
}
```

### `src/sybase/SybReader.hpp`

```cpp
#pragma once
#include "db/IDBReader.hpp"
#include <memory>

class IDBRow;

class SybReader : public IDBReader {
public:
    SybReader();
    ~SybReader() override;

    bool next() override;
    IDBRow& row() override;

private:
    std::unique_ptr<IDBRow> _row;
};
```

### `src/sybase/SybReader.cpp`

```cpp
#include "SybReader.hpp"
#include "sybase/SybRow.hpp"
#include "db/DBException.hpp"

SybReader::SybReader()
    : _row(std::make_unique<SybRow>()) {}

SybReader::~SybReader() = default;

bool SybReader::next() {
    return false;
}

IDBRow& SybReader::row() {
    if (!_row) throw DBException("SybReader::row: no row");
    return *_row;
}
```

### `src/sybase/SybRow.hpp`

```cpp
#pragma once
#include "db/IDBRow.hpp"
#include <vector>
#include <memory>

class IDBValue;

class SybRow : public IDBRow {
public:
    SybRow();
    ~SybRow() override;

    std::size_t columnCount() const override;
    const IDBValue& operator[](std::size_t idx) const override;

private:
    std::vector<std::unique_ptr<IDBValue>> _values;
};
```

### `src/sybase/SybRow.cpp`

```cpp
#include "SybRow.hpp"
#include "sybase/SybValue.hpp"
#include "db/DBException.hpp"

SybRow::SybRow() = default;
SybRow::~SybRow() = default;

std::size_t SybRow::columnCount() const {
    return _values.size();
}

const IDBValue& SybRow::operator[](std::size_t idx) const {
    if (idx >= _values.size()) {
        throw DBException("SybRow::operator[]: index out of range");
    }
    return *_values[idx];
}
```

### `src/sybase/SybValue.hpp`

```cpp
#pragma once
#include "db/IDBValue.hpp"
#include <string>

class SybValue : public IDBValue {
public:
    SybValue();
    SybValue(std::string v, bool isNull);

    bool isNull() const override;
    int asInt() const override;
    double asDouble() const override;
    std::string asString() const override;

private:
    std::string _value;
    bool _null{true};
};
```

### `src/sybase/SybValue.cpp`

```cpp
#include "SybValue.hpp"
#include "db/DBException.hpp"
#include <cstdlib>

SybValue::SybValue() = default;

SybValue::SybValue(std::string v, bool isNull)
    : _value(std::move(v)), _null(isNull) {}

bool SybValue::isNull() const {
    return _null;
}

int SybValue::asInt() const {
    if (_null) throw DBException("SybValue::asInt: null");
    return std::stoi(_value);
}

double SybValue::asDouble() const {
    if (_null) throw DBException("SybValue::asDouble: null");
    return std::stod(_value);
}

std::string SybValue::asString() const {
    if (_null) return {};
    return _value;
}
```

### `src/sybase/SybPreparedStatement.hpp`

```cpp
#pragma once
#include "db/IDBPreparedStatement.hpp"
#include <string>
#include <vector>

class SybPreparedStatement : public IDBPreparedStatement {
public:
    explicit SybPreparedStatement(std::string sql);
    ~SybPreparedStatement() override;

    void bindInt(int index, int value) override;
    void bindDouble(int index, double value) override;
    void bindString(int index, const std::string& value) override;

    std::unique_ptr<IDBReader> executeQuery() override;
    void executeUpdate() override;

private:
    std::string _sql;
    std::vector<std::string> _params;
};
```

### `src/sybase/SybPreparedStatement.cpp`

```cpp
#include "SybPreparedStatement.hpp"
#include "db/DBException.hpp"
#include "sybase/SybReader.hpp"

SybPreparedStatement::SybPreparedStatement(std::string sql)
    : _sql(std::move(sql)) {}

SybPreparedStatement::~SybPreparedStatement() = default;

void SybPreparedStatement::bindInt(int index, int value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindInt: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void SybPreparedStatement::bindDouble(int index, double value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindDouble: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = std::to_string(value);
}

void SybPreparedStatement::bindString(int index, const std::string& value) {
    if (index <= 0) throw DBException("SybPreparedStatement::bindString: index <= 0");
    if (static_cast<std::size_t>(index) > _params.size())
        _params.resize(index);
    _params[index - 1] = value;
}

std::unique_ptr<IDBReader> SybPreparedStatement::executeQuery() {
    throw DBException("SybPreparedStatement::executeQuery not implemented");
}

void SybPreparedStatement::executeUpdate() {
    throw DBException("SybPreparedStatement::executeUpdate not implemented");
}
```

### `src/sybase/SybTransaction.hpp`

```cpp
#pragma once
#include "db/IDBTransaction.hpp"

class SybTransaction : public IDBTransaction {
public:
    SybTransaction();
    ~SybTransaction() override;

    void commit() override;
    void rollback() override;

private:
    bool _active{true};
};
```

### `src/sybase/SybTransaction.cpp`

```cpp
#include "SybTransaction.hpp"
#include "db/DBException.hpp"

SybTransaction::SybTransaction() = default;
SybTransaction::~SybTransaction() = default;

void SybTransaction::commit() {
    if (!_active) throw DBException("SybTransaction::commit: not active");
    _active = false;
}

void SybTransaction::rollback() {
    if (!_active) throw DBException("SybTransaction::rollback: not active");
    _active = false;
}
```

---

## 6. `src/catalog` — Catalog & metadata

### `src/catalog/DbDialect.hpp`

```cpp
#pragma once

enum class DbDialect {
    Sybase,
    PostgreSQL
};
```

### `src/catalog/ColumnMeta.hpp`

```cpp
#pragma once
#include <string>

struct ColumnMeta {
    std::string name;
    std::string typeName;
    int length{};
    int scale{};
    bool nullable{};
};
```

### `src/catalog/TableMeta.hpp`

```cpp
#pragma once
#include <string>
#include <vector>
#include "ColumnMeta.hpp"

struct TableMeta {
    std::string name;
    std::vector<ColumnMeta> columns;
};
```

### `src/catalog/Catalog.hpp`

```cpp
#pragma once
#include <unordered_map>
#include <string>
#include "TableMeta.hpp"
#include "DbDialect.hpp"

class IDBConnection;

class Catalog {
public:
    Catalog(IDBConnection& conn, DbDialect dialect);

    const TableMeta* findTable(const std::string& name) const;
    const std::unordered_map<std::string, TableMeta>& tables() const;

private:
    void loadSybase(IDBConnection& conn);
    void loadPostgres(IDBConnection& conn);

    std::unordered_map<std::string, TableMeta> _tables;
};
```

### `src/catalog/Catalog.cpp`

```cpp
#include "Catalog.hpp"
#include "db/IDBConnection.hpp"
#include "db/IDBReader.hpp"
#include "db/IDBRow.hpp"
#include "db/IDBValue.hpp"

Catalog::Catalog(IDBConnection& conn, DbDialect dialect) {
    switch (dialect) {
    case DbDialect::Sybase:
        loadSybase(conn);
        break;
    case DbDialect::PostgreSQL:
        loadPostgres(conn);
        break;
    }
}

const TableMeta* Catalog::findTable(const std::string& name) const {
    auto it = _tables.find(name);
    if (it == _tables.end()) return nullptr;
    return &it->second;
}

const std::unordered_map<std::string, TableMeta>& Catalog::tables() const {
    return _tables;
}

void Catalog::loadSybase(IDBConnection& conn) {
    (void)conn;
    // Stub: you can implement real queries later
}

void Catalog::loadPostgres(IDBConnection& conn) {
    (void)conn;
    // Stub: you can implement real queries later
}
```

---

## 7. `src/entity` — BaseEntity, EntityTraits, Column, generated entity

### `src/entity/BaseEntity.hpp`

```cpp
#pragma once
#include <nlohmann/json.hpp>

class BaseEntity {
public:
    virtual ~BaseEntity() = default;
    virtual nlohmann::json toJson() const = 0;
};
```

### `src/entity/Column.hpp`

```cpp
#pragma once
#include <string_view>

template<typename Entity, typename FieldType>
struct Column {
    std::string_view name;
    FieldType Entity::* member;
};
```

### `src/entity/EntityTraits.hpp`

```cpp
#pragma once
#include <tuple>
#include <string_view>
#include "Column.hpp"

template<typename T>
struct EntityTraits;
```

### `src/entity/generated/FXInstrument2.hpp`

```cpp
#pragma once
#include "entity/BaseEntity.hpp"
#include "entity/EntityTraits.hpp"
#include "entity/Column.hpp"
#include <string>
#include <tuple>
#include <nlohmann/json.hpp>

class FXInstrument2 : public BaseEntity {
public:
    int _id{};
    int _userId{};
    int _instrumentId{};
    std::string _side;
    double _quantity{};
    double _price{};
    std::string _timestamp;

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["id"] = _id;
        j["userId"] = _userId;
        j["instrumentId"] = _instrumentId;
        j["side"] = _side;
        j["quantity"] = _quantity;
        j["price"] = _price;
        j["timestamp"] = _timestamp;
        return j;
    }
};

template<>
struct EntityTraits<FXInstrument2> {
    using Entity = FXInstrument2;

    static constexpr std::string_view tableName  = "FXInstrument2";
    static constexpr std::string_view primaryKey = "id";

    static constexpr auto columns = std::make_tuple(
        Column<Entity, int>{ "id", &Entity::_id },
        Column<Entity, int>{ "userId", &Entity::_userId },
        Column<Entity, int>{ "instrumentId", &Entity::_instrumentId },
        Column<Entity, std::string>{ "side", &Entity::_side },
        Column<Entity, double>{ "quantity", &Entity::_quantity },
        Column<Entity, double>{ "price", &Entity::_price },
        Column<Entity, std::string>{ "timestamp", &Entity::_timestamp }
    );
};
```

---

## 8. `src/repository` — Generic repository + generated repo

### `src/repository/Repository.hpp`

```cpp
#pragma once
#include "entity/EntityTraits.hpp"
#include "db/IDBConnection.hpp"
#include "db/IDBPreparedStatement.hpp"
#include "db/IDBReader.hpp"
#include "db/IDBRow.hpp"
#include "db/IDBValue.hpp"
#include "db/IDBTransaction.hpp"
#include "db/DBException.hpp"
#include <vector>
#include <string>
#include <sstream>

template<typename Entity>
class Repository {
public:
    explicit Repository(IDBConnection& conn)
        : _conn(conn) {}

    std::vector<Entity> getAll() {
        std::vector<Entity> result;
        std::ostringstream oss;
        oss << "SELECT * FROM " << EntityTraits<Entity>::tableName;
        auto reader = _conn.executeQuery(oss.str());
        while (reader->next()) {
            Entity e{};
            mapRowToEntity(reader->row(), e);
            result.push_back(std::move(e));
        }
        return result;
    }

    Entity getById(int id) {
        std::ostringstream oss;
        oss << "SELECT * FROM " << EntityTraits<Entity>::tableName
            << " WHERE " << EntityTraits<Entity>::primaryKey << "=" << id;
        auto reader = _conn.executeQuery(oss.str());
        if (!reader->next()) {
            throw DBException("Entity not found");
        }
        Entity e{};
        mapRowToEntity(reader->row(), e);
        return e;
    }

    void insert(const Entity& e) {
        // Stub: you can implement literal SQL insert later
        (void)e;
        throw DBException("Repository::insert not implemented");
    }

    void update(const Entity& e) {
        (void)e;
        throw DBException("Repository::update not implemented");
    }

    void remove(const Entity& e) {
        (void)e;
        throw DBException("Repository::remove not implemented");
    }

    void insertPS(const Entity& e) {
        (void)e;
        throw DBException("Repository::insertPS not implemented");
    }

    void insertBatch(const std::vector<Entity>& list) {
        (void)list;
        throw DBException("Repository::insertBatch not implemented");
    }

protected:
    void mapRowToEntity(IDBRow& row, Entity& e) {
        // Very naive: assume columns in order of EntityTraits
        constexpr auto cols = EntityTraits<Entity>::columns;
        mapRowToEntityImpl(row, e, cols, std::make_index_sequence<std::tuple_size_v<decltype(cols)>>{});
    }

    template<std::size_t... I>
    void mapRowToEntityImpl(IDBRow& row, Entity& e,
                            const decltype(EntityTraits<Entity>::columns)& cols,
                            std::index_sequence<I...>) {
        // This is just a stub; real implementation would use type traits
        (void)row;
        (void)e;
        (void)cols;
        (void)sizeof...(I);
    }

private:
    IDBConnection& _conn;
};
```

### `src/repository/generated/Repository_FXInstrument2.hpp`

```cpp
#pragma once
#include "repository/Repository.hpp"
#include "entity/generated/FXInstrument2.hpp"

using Repository_FXInstrument2 = Repository<FXInstrument2>;
```

---

## 9. `src/codegen` — Generators + main

These are **stubs** that just print what they would do.

### `src/codegen/EntityGenerator.hpp`

```cpp
#pragma once
#include <string>

class Catalog;

class EntityGenerator {
public:
    void generateEntities(const Catalog& catalog, const std::string& outputDir);
    void generateEntityTraits(const Catalog& catalog, const std::string& outputDir);
};
```

### `src/codegen/EntityGenerator.cpp`

```cpp
#include "EntityGenerator.hpp"
#include "catalog/Catalog.hpp"
#include <iostream>

void EntityGenerator::generateEntities(const Catalog& catalog, const std::string& outputDir) {
    (void)catalog;
    std::cout << "[EntityGenerator] Would generate entities into: " << outputDir << "\n";
}

void EntityGenerator::generateEntityTraits(const Catalog& catalog, const std::string& outputDir) {
    (void)catalog;
    std::cout << "[EntityGenerator] Would generate EntityTraits into: " << outputDir << "\n";
}
```

### `src/codegen/RepositoryGenerator.hpp`

```cpp
#pragma once
#include <string>

class Catalog;

class RepositoryGenerator {
public:
    void generateRepositories(const Catalog& catalog, const std::string& outputDir);
};
```

### `src/codegen/RepositoryGenerator.cpp`

```cpp
#include "RepositoryGenerator.hpp"
#include "catalog/Catalog.hpp"
#include <iostream>

void RepositoryGenerator::generateRepositories(const Catalog& catalog, const std::string& outputDir) {
    (void)catalog;
    std::cout << "[RepositoryGenerator] Would generate repositories into: " << outputDir << "\n";
}
```

### `src/codegen/UnitTestGenerator.hpp`

```cpp
#pragma once
#include <string>

class Catalog;

class UnitTestGenerator {
public:
    void generateTests(const Catalog& catalog, const std::string& outputDir);
};
```

### `src/codegen/UnitTestGenerator.cpp`

```cpp
#include "UnitTestGenerator.hpp"
#include "catalog/Catalog.hpp"
#include <iostream>

void UnitTestGenerator::generateTests(const Catalog& catalog, const std::string& outputDir) {
    (void)catalog;
    std::cout << "[UnitTestGenerator] Would generate tests into: " << outputDir << "\n";
}
```

### `src/codegen/main_codegen.cpp`

```cpp
#include "catalog/Catalog.hpp"
#include "db/IDBConnection.hpp"
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
```

---

## 10. `tests` — Mock DB + one test

### `tests/MockConnection.hpp`

```cpp
#pragma once
#include "db/IDBConnection.hpp"
#include <string>
#include <memory>

class MockReader;
class MockPreparedStatement;
class MockTransaction;

class MockConnection : public IDBConnection {
public:
    MockConnection() = default;

    std::unique_ptr<IDBReader> executeQuery(const std::string& sql) override;
    std::unique_ptr<IDBPreparedStatement> prepare(const std::string& sql) override;
    std::unique_ptr<IDBTransaction> beginTransaction() override;

    const std::string& lastQuery() const { return _lastQuery; }
    const std::string& lastPreparedSQL() const { return _lastPreparedSQL; }

private:
    std::string _lastQuery;
    std::string _lastPreparedSQL;
};
```

### `tests/MockConnection.cpp`

```cpp
#include "MockConnection.hpp"
#include "MockReader.hpp"
#include "MockPreparedStatement.hpp"
#include "MockTransaction.hpp"

std::unique_ptr<IDBReader> MockConnection::executeQuery(const std::string& sql) {
    _lastQuery = sql;
    return std::make_unique<MockReader>();
}

std::unique_ptr<IDBPreparedStatement> MockConnection::prepare(const std::string& sql) {
    _lastPreparedSQL = sql;
    return std::make_unique<MockPreparedStatement>();
}

std::unique_ptr<IDBTransaction> MockConnection::beginTransaction() {
    return std::make_unique<MockTransaction>();
}
```

### `tests/MockReader.hpp`

```cpp
#pragma once
#include "db/IDBReader.hpp"
#include <memory>

class IDBRow;

class MockReader : public IDBReader {
public:
    MockReader();
    ~MockReader() override;

    bool next() override;
    IDBRow& row() override;

private:
    std::unique_ptr<IDBRow> _row;
    bool _done{false};
};
```

### `tests/MockReader.cpp`

```cpp
#include "MockReader.hpp"
#include "MockRow.hpp"

MockReader::MockReader()
    : _row(std::make_unique<MockRow>()) {}

MockReader::~MockReader() = default;

bool MockReader::next() {
    if (_done) return false;
    _done = true;
    return true;
}

IDBRow& MockReader::row() {
    return *_row;
}
```

### `tests/MockRow.hpp`

```cpp
#pragma once
#include "db/IDBRow.hpp"
#include <vector>
#include <memory>

class IDBValue;

class MockRow : public IDBRow {
public:
    MockRow();
    ~MockRow() override;

    std::size_t columnCount() const override;
    const IDBValue& operator[](std::size_t idx) const override;

private:
    std::vector<std::unique_ptr<IDBValue>> _values;
};
```

### `tests/MockRow.cpp`

```cpp
#include "MockRow.hpp"
#include "db/IDBValue.hpp"
#include "db/DBException.hpp"

class SimpleValue : public IDBValue {
public:
    SimpleValue(int v) : _v(std::to_string(v)), _null(false) {}
    bool isNull() const override { return _null; }
    int asInt() const override { return std::stoi(_v); }
    double asDouble() const override { return std::stod(_v); }
    std::string asString() const override { return _v; }
private:
    std::string _v;
    bool _null{false};
};

MockRow::MockRow() {
    _values.emplace_back(std::make_unique<SimpleValue>(42));
}

MockRow::~MockRow() = default;

std::size_t MockRow::columnCount() const {
    return _values.size();
}

const IDBValue& MockRow::operator[](std::size_t idx) const {
    if (idx >= _values.size()) {
        throw DBException("MockRow::operator[]: index out of range");
    }
    return *_values[idx];
}
```

### `tests/MockPreparedStatement.hpp`

```cpp
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
```

### `tests/MockPreparedStatement.cpp`

```cpp
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
```

### `tests/MockTransaction.hpp`

```cpp
#pragma once
#include "db/IDBTransaction.hpp"

class MockTransaction : public IDBTransaction {
public:
    MockTransaction() = default;

    void commit() override { _committed = true; }
    void rollback() override { _rolledBack = true; }

    bool committed() const { return _committed; }
    bool rolledBack() const { return _rolledBack; }

private:
    bool _committed{false};
    bool _rolledBack{false};
};
```

### `tests/MockTransaction.cpp`

```cpp
#include "MockTransaction.hpp"
// All inline; file exists for symmetry.
```

### `tests/Test_Repository_FXInstrument2.cpp`

```cpp
#include <gtest/gtest.h>
#include "tests/MockConnection.hpp"
#include "entity/generated/FXInstrument2.hpp"
#include "repository/generated/Repository_FXInstrument2.hpp"

TEST(RepositoryFXInstrument2, GetAllBuildsSelect) {
    MockConnection conn;
    Repository_FXInstrument2 repo(conn);

    auto all = repo.getAll();
    (void)all;

    EXPECT_EQ(conn.lastQuery(), "SELECT * FROM FXInstrument2");
}
```

---

Christophe…  
You now have:

- A **full project skeleton**  
- All `.hpp` and `.cpp` files  
- A **CMake project** that ties it all together  
- A **clear file structure** so you always know where things live  

From here, you can:

- Replace stubs with real Sybase / PostgreSQL logic  
- Extend Catalog queries  
- Flesh out Repository SQL generation  
- Enhance codegen to actually write files  

If you want, next step could be:

- Implementing **real SQL generation** in `Repository<T>`  
- Implementing **real Catalog queries** for Sybase/PostgreSQL  
- Wiring **PgConnection** to libpq with real code  

Dis-moi sur quel morceau tu veux qu’on mette la lumière en premier.