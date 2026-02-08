cmake_minimum_required(VERSION 3.15)
project(HFT-LegacyMigrationFramework VERSION 1.0.0 LANGUAGES CXX)

# C++17 required for compile-time reflection
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(WITH_POSTGRESQL "Build with PostgreSQL support" ON)
option(WITH_SYBASE "Build with Sybase DB-Lib support" OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find dependencies
if(WITH_POSTGRESQL)
    find_package(PostgreSQL REQUIRED)
    include_directories(${PostgreSQL_INCLUDE_DIRS})
    add_definitions(-DWITH_POSTGRESQL)
endif()

# Optional: help CMake find a locally-built libpqxx (contains a cmake/ with a
# libpqxx-config.cmake). Provide `-DLIBPQXX_ROOT=E:/Dev/libpqxx-7.10.5` or
# supply the path in `CMAKE_PREFIX_PATH`.
set(LIBPQXX_ROOT "" CACHE PATH "Root directory of libpqxx (e.g. E:/Dev/libpqxx-7.10.5)")

if(LIBPQXX_ROOT)
    # Allow find_package to look under the provided prefix
    list(APPEND CMAKE_PREFIX_PATH "${LIBPQXX_ROOT}")
endif()

# Try to find libpqxx via its CMake config package if available
find_package(libpqxx CONFIG QUIET)
if(TARGET libpqxx::pqxx OR TARGET pqxx::pqxx)
    message(STATUS "Found libpqxx via CMake config")
    add_definitions(-DWITH_LIBPQXX)
else()
    message(STATUS "libpqxx CMake config not found. To specify location use -DLIBPQXX_ROOT=E:/Dev/libpqxx-7.10.5 or -DCMAKE_PREFIX_PATH=path")
endif()

if(WITH_SYBASE)
    # Sybase DB-Lib is typically in /opt/sap or custom location
    find_path(SYBASE_INCLUDE_DIR sybfront.h
        PATHS /opt/sap/OCS-*/include
              /usr/local/include
              /usr/include)
    
    find_library(SYBASE_LIBRARY NAMES sybdb
        PATHS /opt/sap/OCS-*/lib
              /usr/local/lib
              /usr/lib)
    
    if(SYBASE_INCLUDE_DIR AND SYBASE_LIBRARY)
        include_directories(${SYBASE_INCLUDE_DIR})
        add_definitions(-DWITH_SYBASE)
    else()
        message(WARNING "Sybase DB-Lib not found. Disabling Sybase support.")
        set(WITH_SYBASE OFF)
    endif()
endif()

# Source files
set(DB_SOURCES "")

if(WITH_POSTGRESQL)
    list(APPEND DB_SOURCES src/db/PostgreSQLConnection.cpp)
endif()

if(WITH_SYBASE)
    list(APPEND DB_SOURCES src/db/SybaseConnection.cpp)
endif()

# Library target (only create if we have sources)
if(DB_SOURCES)
    add_library(hft-legacy-migration SHARED ${DB_SOURCES})
    
    # Link libraries
    if(WITH_POSTGRESQL)
        target_link_libraries(hft-legacy-migration PRIVATE ${PostgreSQL_LIBRARIES})
        # If libpqxx was found by find_package(libpqxx CONFIG), link its imported target.
        if(TARGET libpqxx::pqxx)
            target_link_libraries(hft-legacy-migration PRIVATE libpqxx::pqxx)
        elseif(TARGET pqxx::pqxx)
            target_link_libraries(hft-legacy-migration PRIVATE pqxx::pqxx)
        endif()
    endif()

    if(WITH_SYBASE)
        target_link_libraries(hft-legacy-migration PRIVATE ${SYBASE_LIBRARY})
    endif()
    
    # Install targets
    install(TARGETS hft-legacy-migration
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
else()
    # Create a header-only interface library when no database backend is enabled
    add_library(hft-legacy-migration INTERFACE)
    message(STATUS "Building as header-only library (no database backends enabled)")
endif()

install(DIRECTORY include/hft DESTINATION include)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
