# Quick Start Guide - Asio SQL Service

This guide will help you quickly build and run the Boost.Asio SQL Service.

## Prerequisites

### Linux
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake libboost-all-dev nlohmann-json3-dev

# Optional: PostgreSQL support
sudo apt-get install -y postgresql-dev libpq-dev libpqxx-dev
```

### Windows
- Install Visual Studio 2017 or later
- Install CMake 3.15+
- Download and install Boost 1.89 (or compatible 1.70+)
- Download nlohmann/json header-only library

## Building

### Linux

```bash
# Navigate to the service directory
cd asio-sql-service

# Create and enter build directory
mkdir build && cd build

# Configure (without database support for testing)
cmake .. -DWITH_POSTGRESQL=OFF -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Binaries will be created:
# - sql_server (server executable)
# - sql_client (client executable)
```

### Windows

```cmd
REM Navigate to the service directory
cd asio-sql-service

REM Create and enter build directory
mkdir build
cd build

REM Configure (adjust BOOST_ROOT to your installation)
cmake .. -DBOOST_ROOT=C:\boost_1_89_0 -DWITH_POSTGRESQL=OFF -G "Visual Studio 16 2019" -A x64

REM Build
cmake --build . --config Release

REM Binaries will be in: build\Release\
```

## Running

### Start the Server

```bash
# Default port (9090)
./sql_server

# Custom port
./sql_server 8080
```

### Use the Client

```bash
# Connect to server
./sql_client localhost 9090

# Then use interactive commands:
sql> raw SELECT * FROM users
sql> json SELECT * FROM products
sql> binary SELECT * FROM orders
sql> stream SELECT * FROM large_table
sql> quit
```

## Testing Without Database

The server includes mock data when no database backend is enabled, so you can test the protocol and communication without setting up a database:

```bash
# Build without database support
cmake .. -DWITH_POSTGRESQL=OFF -DWITH_SYBASE=OFF

# Build and run
cmake --build .
./sql_server &
./sql_client localhost 9090
```

## Quick Tests

```bash
# Start server in background
./sql_server 9090 &

# Test raw query
echo -e "raw SELECT * FROM test\nquit" | ./sql_client localhost 9090

# Test JSON query
echo -e "json SELECT * FROM test\nquit" | ./sql_client localhost 9090

# Test binary query
echo -e "binary SELECT * FROM test\nquit" | ./sql_client localhost 9090

# Test stream query
echo -e "stream SELECT * FROM test\nquit" | ./sql_client localhost 9090

# Stop server
killall sql_server
```

## Building with PostgreSQL Support

```bash
# Install PostgreSQL development libraries
sudo apt-get install -y postgresql-dev libpq-dev libpqxx-dev

# Configure with PostgreSQL
cmake .. -DWITH_POSTGRESQL=ON -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Configure PostgreSQL connection in the server
# (Currently hardcoded, see server/sql_server.hpp line ~120)
```

## Example Output

### Server
```
==================================================
     HFT SQL Service Server (Boost.Asio 1.89)    
==================================================

Starting server on port 9090...
PostgreSQL support: DISABLED (using mock data)
Sybase support: DISABLED

Supported query types:
  - QUERY_RAW: Returns raw rows with column names
  - QUERY_JSON: Returns results as JSON array
  - QUERY_BINARY: Returns binary serialized data
  - QUERY_STREAM: Returns streamed results with metadata

Server listening on port 9090
Press Ctrl+C to stop the server
```

### Client
```
==================================================
     HFT SQL Service Client (Boost.Asio 1.89)    
==================================================

Connecting to localhost:9090...
Connected successfully!

sql> raw SELECT * FROM test

--- Raw Response ---
id | name | value
---+------+------
1 | test1 | 100
2 | test2 | 200
3 | test3 | 300

Total rows: 3
```

## Documentation

For complete documentation, see:
- [README.md](README.md) - Full technical documentation
- [../README.md](../README.md) - Main framework documentation

## Troubleshooting

### Boost Not Found
Set BOOST_ROOT when configuring:
```bash
cmake .. -DBOOST_ROOT=/path/to/boost
```

### nlohmann/json Not Found
```bash
# Ubuntu/Debian
sudo apt-get install nlohmann-json3-dev

# Or download header
mkdir -p include/nlohmann
wget -O include/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
```

### Connection Refused
- Ensure server is running: `ps aux | grep sql_server`
- Check firewall: `sudo ufw status`
- Verify port not in use: `netstat -an | grep 9090`

## Next Steps

1. Review the [full documentation](README.md)
2. Integrate with your existing HFT database layer
3. Add connection pooling for production use
4. Implement authentication and authorization
5. Add TLS/SSL encryption for secure communication

## License

Part of the HFT Legacy Migration Framework. See LICENSE file for details.
