# Asio SQL Service Examples

This directory contains example programs demonstrating how to use the Asio SQL Service.

## Examples

### integration_example.cpp

Demonstrates integration with the HFT framework, showing:
- RAW query execution (IDBRow-compatible)
- JSON query execution (BaseEntity-compatible)
- Binary query execution (optimized)
- Streaming query execution (large datasets)
- Error handling

## Building Examples

Examples are not built by default. To build them, you would need to update CMakeLists.txt:

```cmake
# Add to CMakeLists.txt
add_executable(integration_example
    examples/integration_example.cpp
)

target_link_libraries(integration_example
    ${Boost_LIBRARIES}
)

if(WIN32)
    target_link_libraries(integration_example ws2_32 wsock32)
else()
    target_link_libraries(integration_example pthread)
endif()
```

## Running Examples

1. Start the server:
```bash
./sql_server 9090
```

2. In another terminal, run the example:
```bash
./integration_example
```

## Example Code

All examples follow this basic pattern:

```cpp
#include <boost/asio.hpp>
#include "../client/sql_client.hpp"

int main() {
    boost::asio::io_context io_context;
    asio_sql::SqlClient client(io_context);
    
    // Connect
    client.connect("localhost", "9090");
    
    // Execute queries
    auto response = client.query_json("SELECT * FROM table");
    
    // Process results
    std::cout << response.data.dump(2) << std::endl;
    
    // Disconnect
    client.disconnect();
    
    return 0;
}
```

## More Examples Needed?

If you need more examples, consider adding:
- Connection pooling example
- Multi-threaded client example
- Custom entity serialization
- Prepared statement example
- Transaction handling example
- Error recovery example
- Performance benchmarking example
