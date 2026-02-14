# HFT Asio SQL Service - Technical Documentation

## Overview

The HFT Asio SQL Service is a high-performance client-server system built with Boost.Asio 1.89 that provides remote SQL query execution capabilities. The system supports multiple response formats including raw rows, JSON, binary serialization, and streaming results.

## Architecture

### System Components

```
┌─────────────────────────────────────────────────────────────┐
│                      Client Application                      │
│  ┌────────────────────────────────────────────────────────┐ │
│  │              SqlClient (sql_client.hpp)                │ │
│  │  - query_raw()    - query_json()                      │ │
│  │  - query_binary() - query_stream()                    │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ TCP/IP (Boost.Asio)
                            │ Protocol: MessageHeader + Payload
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                      Server Application                      │
│  ┌────────────────────────────────────────────────────────┐ │
│  │              SqlServer (sql_server.hpp)                │ │
│  │                                                        │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │        Session (per-client handler)              │ │ │
│  │  │  - handle_query_raw()                            │ │ │
│  │  │  - handle_query_json()                           │ │ │
│  │  │  - handle_query_binary()                         │ │ │
│  │  │  - handle_query_stream()                         │ │ │
│  │  └──────────────────────────────────────────────────┘ │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ Database Access
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  Database Layer (HFT Framework)              │
│  - IDBRow / IDBValue abstractions                           │
│  - PostgreSQL / Sybase implementations                       │
│  - Entity traits & ORM support                               │
└─────────────────────────────────────────────────────────────┘
```

### Design Patterns

1. **Asynchronous I/O**: Uses Boost.Asio for non-blocking network operations
2. **Session-per-Connection**: Each client connection gets its own Session object
3. **Protocol Buffer Pattern**: Fixed-size header followed by variable payload
4. **Strategy Pattern**: Multiple query execution strategies (raw, JSON, binary, stream)

## Protocol Specification

### Message Format

All messages follow a simple binary protocol:

```
┌──────────────────┬───────────────────────────────────────┐
│  Message Header  │            Payload                    │
│   (5 bytes)      │         (variable size)               │
└──────────────────┴───────────────────────────────────────┘
```

### Message Header Structure

| Field | Type | Size | Description |
|-------|------|------|-------------|
| message_type | uint8_t | 1 byte | Message type enum value |
| payload_size | uint32_t | 4 bytes | Size of payload in bytes (big-endian) |

### Message Types

#### Request Types
- `QUERY_RAW (1)`: Execute query, return raw rows with column names
- `QUERY_JSON (2)`: Execute query, return results as JSON array
- `QUERY_BINARY (3)`: Execute query, return binary serialized data
- `QUERY_STREAM (4)`: Execute query, return streamed results with metadata

#### Response Types
- `RESPONSE_RAW (11)`: Raw row data response
- `RESPONSE_JSON (12)`: JSON formatted response
- `RESPONSE_BINARY (13)`: Binary data response
- `RESPONSE_STREAM (14)`: Streamed data response
- `RESPONSE_ERROR (99)`: Error response

### Payload Formats

#### Query Request Payload
```
Plain SQL query string (UTF-8 encoded)
```

#### Raw Response Payload (JSON format)
```json
{
  "columns": ["col1", "col2", "col3"],
  "rows": [
    ["value1", "value2", "value3"],
    ["value4", "value5", "value6"]
  ]
}
```

#### JSON Response Payload
```json
[
  {
    "col1": "value1",
    "col2": "value2",
    "col3": "value3"
  },
  {
    "col1": "value4",
    "col2": "value5",
    "col3": "value6"
  }
]
```

#### Binary Response Format
```
[num_rows: 4 bytes][num_cols: 4 bytes]
[cell1_len: 4 bytes][cell1_data]
[cell2_len: 4 bytes][cell2_data]
...
```

All multi-byte integers use big-endian byte order.

#### Stream Response Payload (JSON format)
```json
[
  {
    "type": "metadata",
    "columns": ["col1", "col2", "col3"]
  },
  {
    "type": "row",
    "index": 0,
    "data": ["value1", "value2", "value3"]
  },
  {
    "type": "row",
    "index": 1,
    "data": ["value4", "value5", "value6"]
  },
  {
    "type": "end",
    "total_rows": 2
  }
]
```

#### Error Response Payload (JSON format)
```json
{
  "error": "Error message description"
}
```

## API Reference

### Server API (sql_server.hpp)

#### SqlServer Class

```cpp
class SqlServer {
public:
    SqlServer(boost::asio::io_context& io_context, short port);
};
```

**Constructor Parameters:**
- `io_context`: Boost.Asio I/O context for asynchronous operations
- `port`: TCP port number to listen on (1-65535)

**Example:**
```cpp
boost::asio::io_context io_context;
asio_sql::SqlServer server(io_context, 9090);
io_context.run();
```

#### Session Class (Internal)

Handles individual client connections. Methods:
- `handle_query_raw()`: Process raw query requests
- `handle_query_json()`: Process JSON query requests
- `handle_query_binary()`: Process binary query requests
- `handle_query_stream()`: Process streaming query requests

### Client API (sql_client.hpp)

#### SqlClient Class

```cpp
class SqlClient {
public:
    SqlClient(boost::asio::io_context& io_context);
    
    void connect(const std::string& host, const std::string& port);
    void disconnect();
    
    RawRowResponse query_raw(const std::string& sql);
    JsonResponse query_json(const std::string& sql);
    BinaryResponse query_binary(const std::string& sql);
    JsonResponse query_stream(const std::string& sql);
};
```

**Methods:**

##### connect()
Establishes connection to the server.

```cpp
void connect(const std::string& host, const std::string& port);
```

**Parameters:**
- `host`: Server hostname or IP address
- `port`: Server port number

**Example:**
```cpp
client.connect("localhost", "9090");
```

##### disconnect()
Closes the connection to the server.

```cpp
void disconnect();
```

##### query_raw()
Executes a SQL query and returns raw rows.

```cpp
RawRowResponse query_raw(const std::string& sql);
```

**Parameters:**
- `sql`: SQL query string

**Returns:** `RawRowResponse` containing:
- `column_names`: Vector of column names
- `rows`: Vector of rows (each row is a vector of strings)

**Example:**
```cpp
auto response = client.query_raw("SELECT * FROM users");
for (const auto& row : response.rows) {
    for (const auto& cell : row) {
        std::cout << cell << " ";
    }
    std::cout << std::endl;
}
```

##### query_json()
Executes a SQL query and returns results as JSON.

```cpp
JsonResponse query_json(const std::string& sql);
```

**Parameters:**
- `sql`: SQL query string

**Returns:** `JsonResponse` containing:
- `data`: nlohmann::json object with array of row objects

**Example:**
```cpp
auto response = client.query_json("SELECT * FROM users");
std::cout << response.data.dump(2) << std::endl;
```

##### query_binary()
Executes a SQL query and returns binary serialized data.

```cpp
BinaryResponse query_binary(const std::string& sql);
```

**Parameters:**
- `sql`: SQL query string

**Returns:** `BinaryResponse` containing:
- `data`: Vector of bytes in custom binary format

**Example:**
```cpp
auto response = client.query_binary("SELECT * FROM users");
// Parse binary data according to format specification
```

##### query_stream()
Executes a SQL query and returns streamed results.

```cpp
JsonResponse query_stream(const std::string& sql);
```

**Parameters:**
- `sql`: SQL query string

**Returns:** `JsonResponse` containing:
- `data`: JSON array with metadata, row chunks, and end marker

**Example:**
```cpp
auto response = client.query_stream("SELECT * FROM large_table");
for (const auto& chunk : response.data) {
    std::string type = chunk["type"];
    if (type == "row") {
        // Process row data
    }
}
```

## Building the Project

### Prerequisites

- **C++17 Compiler**:
  - GCC 7+ on Linux
  - MSVC 2017+ on Windows
  - Clang 5+ on macOS

- **CMake 3.15+**

- **Boost 1.89** (or compatible version 1.70+)
  - Components: system

- **nlohmann/json** (header-only library)

- **Optional**: PostgreSQL client libraries (libpq, libpqxx) for database support

### Build Instructions

#### Linux Build

```bash
# Navigate to asio-sql-service directory
cd asio-sql-service

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBOOST_ROOT=/usr/local \
  -DWITH_POSTGRESQL=ON

# Build
cmake --build . -j$(nproc)

# Binaries will be in build directory:
# - sql_server
# - sql_client
```

#### Windows Build

```cmd
REM Navigate to asio-sql-service directory
cd asio-sql-service

REM Create build directory
mkdir build
cd build

REM Configure with CMake
cmake .. ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DBOOST_ROOT=C:/boost_1_89_0 ^
  -DWITH_POSTGRESQL=OFF ^
  -G "Visual Studio 16 2019" ^
  -A x64

REM Build
cmake --build . --config Release

REM Binaries will be in build/Release:
REM - sql_server.exe
REM - sql_client.exe
```

### CMake Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `BOOST_ROOT` | (empty) | Path to Boost installation |
| `WITH_POSTGRESQL` | ON | Enable PostgreSQL support |
| `WITH_SYBASE` | OFF | Enable Sybase support |
| `LIBPQXX_ROOT` | (empty) | Path to libpqxx installation |
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug/Release) |

### Build Without Database Support

For testing purposes, you can build without any database backend:

```bash
cmake .. -DWITH_POSTGRESQL=OFF -DWITH_SYBASE=OFF
cmake --build .
```

The server will use mock data when no database backend is available.

## Usage Examples

### Starting the Server

#### Default Port (9090)
```bash
./sql_server
```

#### Custom Port
```bash
./sql_server 8080
```

#### Server Output
```
==================================================
     HFT SQL Service Server (Boost.Asio 1.89)    
==================================================

Starting server on port 9090...
PostgreSQL support: ENABLED
Sybase support: DISABLED

Supported query types:
  - QUERY_RAW: Returns raw rows with column names
  - QUERY_JSON: Returns results as JSON array
  - QUERY_BINARY: Returns binary serialized data
  - QUERY_STREAM: Returns streamed results with metadata

Server listening on port 9090
Press Ctrl+C to stop the server
```

### Using the Client

#### Interactive Mode (Default)
```bash
./sql_client localhost 9090
```

#### Interactive Commands
```
sql> raw SELECT * FROM users WHERE id < 10
sql> json SELECT name, email FROM users
sql> binary SELECT * FROM large_table
sql> stream SELECT * FROM products
sql> quit
```

#### Example Session
```
==================================================
     HFT SQL Service Client (Boost.Asio 1.89)    
==================================================

Connecting to localhost:9090...
Connected successfully!

sql> raw SELECT id, name, email FROM users LIMIT 3

--- Raw Response ---
id | name | email
---+------+-------
1  | John | john@example.com
2  | Jane | jane@example.com
3  | Bob  | bob@example.com

Total rows: 3

sql> json SELECT id, name FROM users LIMIT 2

--- JSON Response ---
[
  {
    "id": "1",
    "name": "John"
  },
  {
    "id": "2",
    "name": "Jane"
  }
]

sql> quit
Goodbye!
```

### Programmatic Client Usage

```cpp
#include <boost/asio.hpp>
#include "sql_client.hpp"

int main() {
    try {
        boost::asio::io_context io_context;
        asio_sql::SqlClient client(io_context);
        
        // Connect to server
        client.connect("localhost", "9090");
        
        // Execute raw query
        auto raw_response = client.query_raw("SELECT * FROM users");
        std::cout << "Rows: " << raw_response.rows.size() << std::endl;
        
        // Execute JSON query
        auto json_response = client.query_json("SELECT * FROM products");
        std::cout << json_response.data.dump(2) << std::endl;
        
        // Execute binary query
        auto binary_response = client.query_binary("SELECT * FROM orders");
        std::cout << "Binary data size: " << binary_response.data.size() << std::endl;
        
        // Execute streaming query
        auto stream_response = client.query_stream("SELECT * FROM large_table");
        for (const auto& chunk : stream_response.data) {
            if (chunk["type"] == "row") {
                // Process row
            }
        }
        
        client.disconnect();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Integration with HFT Framework

### Using Existing Database Abstractions

The server integrates with the HFT Legacy Migration Framework's database layer:

```cpp
// Server-side query execution with PostgreSQL
#ifdef WITH_POSTGRESQL
    auto stmt = db_connection_->createStatement(sql);
    auto result = stmt->executeQuery();
    
    while (result->next()) {
        // Access via IResultSet interface
        int id = result->getInt(0);
        std::string name = result->getString(1);
        // ...
    }
#endif
```

### Entity Support

When using entities derived from `BaseEntity` with `EntityTraits`:

```cpp
// Example entity
class User : public BaseEntity {
public:
    int id;
    std::string name;
    std::string email;
    
    nlohmann::json toJson() const override {
        return {
            {"id", id},
            {"name", name},
            {"email", email}
        };
    }
};

// EntityTraits specialization
template<>
struct EntityTraits<User> {
    static constexpr std::string_view tableName = "users";
    static constexpr auto columns = std::make_tuple(
        Column<User, int>{"id", &User::id},
        Column<User, std::string>{"name", &User::name},
        Column<User, std::string>{"email", &User::email}
    );
};
```

The JSON query method automatically supports entities that implement `toJson()`.

### Using IDBRow and IDBValue

The raw query method returns data compatible with `IDBRow` and `IDBValue` interfaces:

```cpp
// Client receives RawRowResponse
auto response = client.query_raw("SELECT * FROM table");

// Can be adapted to IDBRow/IDBValue if needed
for (size_t row_idx = 0; row_idx < response.rows.size(); ++row_idx) {
    for (size_t col_idx = 0; col_idx < response.rows[row_idx].size(); ++col_idx) {
        std::string value = response.rows[row_idx][col_idx];
        // Process value
    }
}
```

## Performance Considerations

### Asynchronous I/O
- Server uses Boost.Asio's async operations for non-blocking I/O
- Each client connection runs in its own asynchronous session
- Multiple clients can be served concurrently

### Memory Management
- Uses `shared_ptr` for session management
- Automatic cleanup when client disconnects
- Buffer reuse within sessions

### Query Optimization
- For large result sets, use `QUERY_STREAM` to avoid loading all data into memory
- Binary format reduces serialization overhead
- Connection pooling can be added for production use

### Scalability
- Single-threaded event loop (can be extended to multi-threaded)
- Connection pool not implemented (should be added for production)
- No query timeout implemented (should be added for production)

## Security Considerations

### SQL Injection
⚠️ **Warning**: This implementation passes SQL queries directly to the database without sanitization. In production:
- Use prepared statements on the server side
- Implement query whitelisting
- Add input validation
- Use parameter binding

### Network Security
The current implementation uses plain TCP without encryption:
- For production, implement TLS/SSL using Boost.Asio SSL
- Add authentication mechanism
- Implement authorization/access control
- Use firewall rules to restrict access

### Error Handling
- Server returns error messages to client
- Avoid leaking sensitive information in error messages
- Log security-relevant events

## Troubleshooting

### Common Issues

#### Boost Not Found
```
CMake Error: Could not find Boost
```

**Solution**: Set `BOOST_ROOT` environment variable or CMake option:
```bash
cmake .. -DBOOST_ROOT=/path/to/boost
```

#### nlohmann/json Not Found
```
fatal error: nlohmann/json.hpp: No such file or directory
```

**Solution**: Install nlohmann/json:
```bash
# Ubuntu/Debian
sudo apt-get install nlohmann-json3-dev

# Or download header-only version to include/
wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
mkdir -p include/nlohmann
mv json.hpp include/nlohmann/
```

#### PostgreSQL Not Found (When WITH_POSTGRESQL=ON)
```
Could not find PostgreSQL
```

**Solution**: Install PostgreSQL development libraries:
```bash
# Ubuntu/Debian
sudo apt-get install postgresql-dev libpq-dev

# Or build without PostgreSQL
cmake .. -DWITH_POSTGRESQL=OFF
```

#### Connection Refused
```
Error: Connection refused
```

**Solution**:
- Ensure server is running: `./sql_server`
- Check firewall settings
- Verify port is not in use: `netstat -an | grep 9090`

#### Server Crashes on Query
```
Segmentation fault (core dumped)
```

**Solution**:
- Check if database connection is properly configured
- Build with debug symbols: `cmake .. -DCMAKE_BUILD_TYPE=Debug`
- Run with debugger: `gdb ./sql_server`

## Extending the System

### Adding New Query Types

1. Add new message type to `protocol.hpp`:
```cpp
enum class MessageType : uint8_t {
    // ...
    QUERY_CUSTOM = 5,
    RESPONSE_CUSTOM = 15
};
```

2. Implement handler in `sql_server.hpp`:
```cpp
void handle_query_custom(const std::string& payload) {
    // Custom implementation
}
```

3. Add client method in `sql_client.hpp`:
```cpp
CustomResponse query_custom(const std::string& sql) {
    // Custom implementation
}
```

### Adding Connection Pooling

Modify `Session` class to use a connection pool:

```cpp
class ConnectionPool {
    std::vector<std::shared_ptr<PostgreSQLConnection>> connections_;
    std::mutex mutex_;
    // Implementation
};

class Session {
    std::shared_ptr<ConnectionPool> pool_;
    // Use pool_->acquire() / pool_->release()
};
```

### Adding Authentication

1. Add authentication request/response messages
2. Implement authentication handler
3. Store session credentials
4. Validate credentials on each query

## License

This component is part of the HFT Legacy Migration Framework. See LICENSE file for details.

## Support

For issues and questions:
- GitHub Issues: [Repository Issues](https://github.com/ChristophePichaud/HFT-LegacyMigrationFramework/issues)
- Documentation: See main framework README.md

## Version History

- **1.0.0** (2024): Initial release
  - Basic client-server implementation
  - Four query types: raw, JSON, binary, stream
  - PostgreSQL and Sybase support
  - Cross-platform (Windows/Linux)
  - ISO C++17 compliant
