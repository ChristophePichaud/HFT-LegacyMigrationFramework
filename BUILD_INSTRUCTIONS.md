# Legacy Migration Framework - Build Instructions

## Overview

This is a complete C++17 project implementing a Legacy Migration Framework with database abstraction, ORM-like entity management, code generation tools, and comprehensive unit testing.

## Project Structure

```
legacy-migration-framework/
├── CMakeLists.txt              # Main build configuration
├── include/                     # Header-only libraries
│   ├── cxxopts/                # Command-line parsing (v3.1.1)
│   └── nlohmann/               # JSON library (v3.11.2)
├── src/
│   ├── db/                     # Database abstraction layer (interfaces)
│   ├── pg/                     # PostgreSQL backend implementation
│   ├── sybase/                 # Sybase DB-Lib backend implementation
│   ├── catalog/                # Schema metadata management
│   ├── entity/                 # Entity base classes and generated entities
│   ├── repository/             # Generic repository pattern + generated repos
│   └── codegen/                # Code generation tools
└── tests/                      # Mock implementations and unit tests
```

## Prerequisites

- **CMake** 3.16 or higher
- **C++17 compatible compiler** (GCC 7+, Clang 5+, MSVC 2017+)
- **Optional**: PostgreSQL development libraries (if USE_POSTGRESQL=ON)
- **Optional**: Sybase DB-Lib (if USE_SYBASE=ON)

## Build Instructions

### Basic Build (Core functionality only)

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake (PostgreSQL and Sybase disabled by default)
cmake .. -DUSE_POSTGRESQL=OFF -DUSE_SYBASE=OFF -DBUILD_TESTS=ON

# Build the project
make -j$(nproc)

# Run tests
./unit_tests

# Run code generator
./lmf_codegen
```

### Build with PostgreSQL Support

```bash
mkdir build && cd build
cmake .. -DUSE_POSTGRESQL=ON -DUSE_SYBASE=OFF -DBUILD_TESTS=ON
make -j$(nproc)
```

### Build with Sybase Support

```bash
mkdir build && cd build
cmake .. -DUSE_POSTGRESQL=OFF -DUSE_SYBASE=ON -DBUILD_TESTS=ON
make -j$(nproc)
```

### Windows Build (PowerShell / Visual Studio)

Use the Visual Studio generator for MSVC toolchains (recommended on Windows).

```powershell
# Configure for Visual Studio 2022 x64 (point to local libpqxx install)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DLIBPQXX_ROOT="E:/Dev/libpqxx-7.10.5" -DWITH_POSTGRESQL=ON -DBUILD_TESTS=ON

# Build Release
cmake --build build --config Release

# Run tests
ctest --test-dir build -C Release --output-on-failure
```

If you prefer Ninja (fast iterative builds) with MSVC toolchain:

```powershell
cmake -S . -B build -G "Ninja" -A x64 -DLIBPQXX_ROOT="E:/Dev/libpqxx-7.10.5" -DWITH_POSTGRESQL=ON
cmake --build build --config Release
```

If `libpqxx` was installed with a CMake package config (`libpqxx-config.cmake`), you can alternatively set `CMAKE_PREFIX_PATH`:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="E:/Dev/libpqxx-7.10.5" -DWITH_POSTGRESQL=ON
```

Windows-specific notes:
- On MSVC you link against `.lib` import libraries and need matching `.dll` files at runtime (for example `libpq.dll`).
- Ensure libpq/libpqxx were built with the same architecture (x64 vs x86) and runtime (MD/MT) as your project.
- Add the PostgreSQL `bin` folder (or the folder containing `libpqxx`/`libpq` DLLs) to `PATH` when running executables or tests.


### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTS` | ON | Build unit tests with GoogleTest |
| `USE_POSTGRESQL` | OFF | Enable PostgreSQL backend |
| `USE_SYBASE` | OFF | Enable Sybase DB-Lib backend |

## Build Artifacts

After a successful build, you'll find:

- **libcore.a** - Core library with DB abstraction and entity management
- **libpg_backend.a** - PostgreSQL backend (if enabled)
- **libsybase_backend.a** - Sybase backend (if enabled)
- **lmf_codegen** - Code generation executable
- **unit_tests** - Unit test suite

## Running Tests

```bash
cd build

# Run all tests
./unit_tests

# Run with verbose output
./unit_tests --gtest_verbose

# List available tests
./unit_tests --gtest_list_tests

# Run specific test
./unit_tests --gtest_filter=RepositoryFXInstrument2.*
```

## Using the Code Generator

The `lmf_codegen` tool generates entity classes, repositories, and unit tests based on database schema:

```bash
cd build
./lmf_codegen
```

Output:
```
[EntityGenerator] Would generate entities into: src/entity/generated
[EntityGenerator] Would generate EntityTraits into: src/entity
[RepositoryGenerator] Would generate repositories into: src/repository/generated
[UnitTestGenerator] Would generate tests into: tests
Code generation stub completed.
```

## Project Features

### 1. Database Abstraction Layer
- Interface-based design (pure virtual classes)
- Support for multiple database backends
- Prepared statements and parameter binding
- Transaction management
- Row-based data access

### 2. Entity System
- Compile-time reflection via EntityTraits
- JSON serialization with nlohmann::json
- Type-safe column definitions
- Generated entity classes

### 3. Repository Pattern
- Generic Repository<T> template
- Type-safe CRUD operations
- Automatic SQL generation
- Connection abstraction

### 4. Code Generation
- Entity class generation from schema
- Repository generation
- Unit test scaffolding

### 5. Testing Infrastructure
- Mock implementations for all DB interfaces
- GoogleTest integration
- Comprehensive unit test coverage

## Example Usage

### Using a Repository

```cpp
#include "entity/generated/FXInstrument2.hpp"
#include "repository/generated/Repository_FXInstrument2.hpp"
#include "pg/PgConnection.hpp"

int main() {
    // Create connection
    PgConnection conn;
    conn.open("postgresql://localhost/mydb");
    
    // Create repository
    Repository_FXInstrument2 repo(conn);
    
    // Fetch all entities
    auto instruments = repo.getAll();
    
    // Find by ID
    auto instrument = repo.findById(42);
    
    // Save entity
    FXInstrument2 newInstrument;
    newInstrument.name = "EUR/USD";
    repo.save(newInstrument);
    
    return 0;
}
```

## Troubleshooting

### Build Errors

1. **"Cannot find nlohmann/json.hpp"**
   - Ensure the `include/` directory is properly configured
   - Check that nlohmann/json.hpp was downloaded correctly

2. **"PostgreSQL not found"**
   - Install PostgreSQL development libraries:
     - Ubuntu/Debian: `sudo apt-get install libpq-dev`
     - RedHat/CentOS: `sudo yum install postgresql-devel`
   - Or build with `-DUSE_POSTGRESQL=OFF`

3. **"Sybase DB-Lib not found"**
   - Install Sybase Open Client
   - Or build with `-DUSE_SYBASE=OFF`

### Test Failures

If tests fail:
```bash
# Run with verbose output
./unit_tests --gtest_verbose

# Check for specific failures
./unit_tests --gtest_filter=*FailingTest*
```

### Windows Troubleshooting

- **CMake did not find libpqxx**: verify the `libpqxx-config.cmake` file exists under `E:/Dev/libpqxx-7.10.5` (common locations: `cmake/`, `lib/cmake/libpqxx/`). If present, use `-DLIBPQXX_ROOT="E:/Dev/libpqxx-7.10.5"` or `-DCMAKE_PREFIX_PATH` to point CMake at it.
- **Missing DLL at runtime**: locate `libpq.dll` (PostgreSQL client) and add its folder to `PATH` or copy the DLL next to the executable.
- **Linker unresolved symbols**: ensure you are linking the correct import library for your toolchain (`.lib` for MSVC, `.a` for MinGW) and that the library was built with the same runtime flags.
- **Exported target name differs**: open the `libpqxx` config file and search for exported target names (`libpqxx::pqxx` or `pqxx::pqxx`)—if it uses a different name, update `CMakeLists.txt` accordingly.

## Development

### Adding New Entity Types

1. Define entity in `src/entity/generated/`
2. Define EntityTraits specialization
3. Create repository alias in `src/repository/generated/`
4. Add unit tests in `tests/`

### Extending Database Backends

1. Implement all interfaces from `src/db/`
2. Add backend sources to CMakeLists.txt
3. Create backend-specific tests

## License

See LICENSE file in repository root.

## Contributing

Please see CONTRIBUTING.md for guidelines on contributing to this project.
