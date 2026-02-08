# HFT-Demo - Comprehensive Demo Application

## Overview

HFT-Demo is a comprehensive console application that demonstrates all the features of the HFT Legacy Migration Framework. It provides command-line tools to test database connections, catalog functionality, DBReader operations, and JSON export capabilities for both PostgreSQL and Sybase databases.

## Features

### 1. Database Connection Testing
- **PostgreSQL Connection Test**: Test connection with user, password, and database name
- **Sybase Connection Test**: Test connection with user, password, and server name
- **Table Count**: Automatically counts and displays the number of tables in the database

### 2. Catalog Functionality
- **List All Tables**: Display all tables in the connected database
- **Column Details**: Show detailed information about columns with the `-d` flag
  - Column names
  - Data types
  - Length/size
  - Nullable constraints

### 3. DBReader Testing
- **Table Creation**: Creates a sample `products` table
- **Data Insertion**: Inserts test product records
- **SELECT * Query**: Retrieves all data using DBReader pattern
- **Formatted Output**: Displays data in a readable tabular format

### 4. JSON Export
- **Table to JSON**: Export any table's contents to JSON format
- **Entity Generation**: Automatically generates entity metadata
- **Custom Output**: Specify output file with `-o` or `--output` flag
- **Pretty Printing**: JSON output is formatted with 2-space indentation

## Building

The HFT-Demo application is built automatically when `BUILD_EXAMPLES=ON` is set in CMake:

```bash
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON -DWITH_POSTGRESQL=ON -DWITH_SYBASE=OFF
make HFT-Demo
```

The executable will be located in `build/examples/HFT-Demo`.

## Usage

### Command-Line Options

| Option | Alias | Description |
|--------|-------|-------------|
| `--sybase` | `-s` | Use Sybase database |
| `--postgresql` | `-p` | Use PostgreSQL database |
| `--user` | `-u` | Database user |
| `--password` | `-w` | Database password |
| `--host` | - | PostgreSQL host (default: localhost) |
| `--server` | - | Sybase server name |
| `--dbname` | - | PostgreSQL database name |
| `--test-connection` | - | Test database connection |
| `--test-catalog` | - | Test catalog functionality |
| `--test-dbreader` | - | Test DBReader functionality |
| `--test-json` | - | Test JSON export (specify table name) |
| `--details` | `-d` | Show detailed information (for catalog) |
| `--output` | `-o` | Output file path (default: output.json) |
| `--help` | `-h` | Print usage information |

## Examples

### 1. Test PostgreSQL Connection

```bash
./HFT-Demo -p --test-connection -u postgres -w password --dbname testdb
```

**Output:**
```
========================================
  Testing PostgreSQL Connection
========================================
Connecting to PostgreSQL database: testdb
✓ Connection successful!

Counting tables in database...
✓ Total tables: 5
```

### 2. Test Sybase Connection

```bash
./HFT-Demo -s --test-connection -u sa -w password --server SYBASE_SERVER
```

### 3. List Database Tables with Details

```bash
./HFT-Demo -p --test-catalog -d -u postgres -w password --dbname testdb
```

**Output:**
```
========================================
  Testing Catalog Functionality
========================================
Fetching table list from postgresql...

--- Tables ---
  • products
    Columns:
      - id (bigint, nullable: NO)
      - name (text, nullable: NO)
      - description (text, nullable: YES)
      - price (double precision, nullable: NO)
      - quantity (integer, nullable: NO)
  • users
    Columns:
      - id (bigint, nullable: NO)
      - name (text, nullable: NO)
      - email (text, nullable: NO)

✓ Total tables found: 2
```

### 4. Test DBReader with Sample Data

```bash
./HFT-Demo -p --test-dbreader -u postgres -w password --dbname testdb
```

**Output:**
```
========================================
  Testing DBReader
========================================
Creating products table...
✓ Table created

Inserting test products...
✓ 3 products inserted

Reading products using SELECT *...

ID   Name           Description                   Price     Quantity
----------------------------------------------------------------------
1    Laptop         High-performance laptop       1299.99   5
2    Mouse          Wireless mouse                29.99     50
3    Keyboard       Mechanical keyboard           89.99     20

✓ DBReader test completed
```

### 5. Export Table to JSON

```bash
./HFT-Demo -p --test-json products -o products.json -u postgres -w password --dbname testdb
```

**Output:**
```
========================================
  Testing JSON Export
========================================
Exporting table 'products' to JSON...
✓ Exported 3 records to: products.json
✓ JSON export completed
```

**Generated JSON (products.json):**
```json
{
  "table": "products",
  "record_count": 3,
  "data": [
    {
      "id": "1",
      "name": "Laptop",
      "description": "High-performance laptop",
      "price": "1299.99",
      "quantity": "5"
    },
    {
      "id": "2",
      "name": "Mouse",
      "description": "Wireless mouse",
      "price": "29.99",
      "quantity": "50"
    },
    {
      "id": "3",
      "name": "Keyboard",
      "description": "Mechanical keyboard",
      "price": "89.99",
      "quantity": "20"
    }
  ]
}
```

## Database Requirements

### PostgreSQL
- PostgreSQL server running and accessible
- Valid database created
- User with appropriate permissions

### Sybase
- Sybase ASE server running and accessible
- Valid database created
- User with appropriate permissions
- Sybase DB-Lib libraries installed
- Framework compiled with `-DWITH_SYBASE=ON`

## Error Handling

The application provides clear error messages for common issues:

- **Missing parameters**: Indicates which parameters are required
- **Connection failures**: Displays the error message from the database
- **Missing database support**: Informs if PostgreSQL or Sybase support is not compiled in

## Implementation Details

### Technologies Used
- **cxxopts**: Command-line argument parsing
- **nlohmann/json**: JSON serialization and deserialization
- **HFT Framework**: Database abstraction, ORM, and catalog functionality

### Product Entity
The application uses a `Product` entity for testing:

```cpp
struct Product {
    int64_t id;
    std::string name;
    std::string description;
    double price;
    int32_t quantity;
};
```

### Database Abstraction
All database operations use the HFT Framework's abstract interfaces:
- `IConnection`: Database connection interface
- `IStatement`: Prepared statement interface
- `IResultSet`: Query result interface
- `Repository<T>`: ORM repository pattern

## Integration with Existing Examples

The HFT-Demo application incorporates functionality from all existing examples:
- `basic_usage.cpp`: Connection handling and basic queries
- `orm_usage.cpp`: ORM repository pattern
- `codegen_usage.cpp`: Entity and table metadata generation

## Future Enhancements

Potential improvements for HFT-Demo:
- Support for multiple output formats (CSV, XML)
- Batch operations support
- Migration testing between Sybase and PostgreSQL
- Performance benchmarking mode
- Interactive mode for multiple operations
- Configuration file support

## See Also

- [Main README](../README.md)
- [Build Instructions](../BUILD_INSTRUCTIONS.md)
- [Developer Guide](../DEVELOPER_GUIDE.md)
