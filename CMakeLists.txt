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
include_directories(${CMAKE_SOURCE_DIR}/include/hft)

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

# Add all .cpp files from src except pg and sybase folders
file(GLOB_RECURSE ALL_SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE PG_SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/pg/*.cpp"
)
file(GLOB_RECURSE SYBASE_SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/sybase/*.cpp"
)


# Add files from src/pg folder
list(APPEND DB_SOURCES ${ALL_SRC_FILES} ${PG_SRC_FILES})

# Only add Sybase sources if WITH_SYBASE is ON
if(WITH_SYBASE)
    list(APPEND DB_SOURCES ${SYBASE_SRC_FILES})
endif()

# Add PostgreSQL and Sybase specific files if enabled
if(WITH_POSTGRESQL)
endif()

if(WITH_SYBASE)
endif()



# Library targets (only create if we have sources)
if(DB_SOURCES)
    # Shared library (.dll/.so/.dylib)
    add_library(hft-legacy-migration SHARED ${DB_SOURCES})
    # Static library (.lib/.a)
    add_library(hft-legacy-migration-static STATIC ${DB_SOURCES})
    set_target_properties(hft-legacy-migration-static PROPERTIES OUTPUT_NAME "hft-legacy-migration")

    # Link libraries for both targets
    foreach(_target hft-legacy-migration hft-legacy-migration-static)
        if(WITH_POSTGRESQL)
            target_link_libraries(${_target} PRIVATE ${PostgreSQL_LIBRARIES})
            if(TARGET libpqxx::pqxx)
                target_link_libraries(${_target} PRIVATE libpqxx::pqxx)
            elseif(TARGET pqxx::pqxx)
                target_link_libraries(${_target} PRIVATE pqxx::pqxx)
            endif()
        endif()
        if(WITH_SYBASE)
            target_link_libraries(${_target} PRIVATE ${SYBASE_LIBRARY})
        endif()
    endforeach()

    # Install targets
    install(TARGETS hft-legacy-migration hft-legacy-migration-static
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
