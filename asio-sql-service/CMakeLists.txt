cmake_minimum_required(VERSION 3.15)
project(AsioSqlService VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(WITH_POSTGRESQL "Build with PostgreSQL support" ON)
option(WITH_SYBASE "Build with Sybase DB-Lib support" OFF)

# Boost configuration - parameterized root
set(BOOST_ROOT "" CACHE PATH "Boost root directory (e.g., C:/boost_1_89_0 or /usr/local)")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include" CACHE PATH "Boost include directory")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib" CACHE PATH "Boost library directory")

# If BOOST_ROOT is not set, try to use default include/boost
if(NOT BOOST_ROOT)
    set(BOOST_INCLUDEDIR "${CMAKE_SOURCE_DIR}/../include/boost")
endif()

# Find Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.70 REQUIRED COMPONENTS system)

if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
else()
    message(FATAL_ERROR "Boost not found. Please set BOOST_ROOT or ensure Boost is installed.")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/common)
include_directories(${CMAKE_SOURCE_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/../include/hft)
include_directories(${Boost_INCLUDE_DIRS})

# Platform-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7 or later
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Find nlohmann/json (header-only)
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
    PATHS
        ${CMAKE_SOURCE_DIR}/../include
        /usr/include
        /usr/local/include
        C:/vcpkg/installed/x64-windows/include
)

if(NLOHMANN_JSON_INCLUDE_DIR)
    message(STATUS "Found nlohmann/json: ${NLOHMANN_JSON_INCLUDE_DIR}")
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
else()
    message(WARNING "nlohmann/json not found. Please ensure it's available.")
endif()

# PostgreSQL support
if(WITH_POSTGRESQL)
    find_package(PostgreSQL REQUIRED)
    include_directories(${PostgreSQL_INCLUDE_DIRS})
    add_definitions(-DWITH_POSTGRESQL)
    
    # Try to find libpqxx
    set(LIBPQXX_ROOT "" CACHE PATH "Root directory of libpqxx")
    if(LIBPQXX_ROOT)
        list(APPEND CMAKE_PREFIX_PATH "${LIBPQXX_ROOT}")
    endif()
    
    find_package(libpqxx CONFIG QUIET)
    if(TARGET libpqxx::pqxx OR TARGET pqxx::pqxx)
        message(STATUS "Found libpqxx via CMake config")
        add_definitions(-DWITH_LIBPQXX)
    endif()
endif()

# Sybase support
if(WITH_SYBASE)
    find_path(SYBASE_INCLUDE_DIR sybfront.h
        PATHS /opt/sap/OCS-*/include /usr/local/include /usr/include)
    
    find_library(SYBASE_LIBRARY NAMES sybdb
        PATHS /opt/sap/OCS-*/lib /usr/local/lib /usr/lib)
    
    if(SYBASE_INCLUDE_DIR AND SYBASE_LIBRARY)
        include_directories(${SYBASE_INCLUDE_DIR})
        add_definitions(-DWITH_SYBASE)
        message(STATUS "Sybase support enabled")
    else()
        message(WARNING "Sybase DB-Lib not found. Disabling Sybase support.")
        set(WITH_SYBASE OFF)
    endif()
endif()

# Get HFT library sources if WITH_POSTGRESQL is enabled
set(HFT_SOURCES "")
if(WITH_POSTGRESQL)
    file(GLOB_RECURSE HFT_DB_SOURCES
        "${CMAKE_SOURCE_DIR}/../src/db/*.cpp"
        "${CMAKE_SOURCE_DIR}/../src/pg/*.cpp"
    )
    list(APPEND HFT_SOURCES ${HFT_DB_SOURCES})
endif()

# Server executable
add_executable(sql_server
    server/server_main.cpp
    ${HFT_SOURCES}
)

target_link_libraries(sql_server
    ${Boost_LIBRARIES}
)

if(WITH_POSTGRESQL)
    target_link_libraries(sql_server ${PostgreSQL_LIBRARIES})
    if(TARGET libpqxx::pqxx)
        target_link_libraries(sql_server libpqxx::pqxx)
    elseif(TARGET pqxx::pqxx)
        target_link_libraries(sql_server pqxx::pqxx)
    endif()
endif()

if(WITH_SYBASE)
    target_link_libraries(sql_server ${SYBASE_LIBRARY})
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(sql_server ws2_32 wsock32)
else()
    target_link_libraries(sql_server pthread)
endif()

# Client executable
add_executable(sql_client
    client/client_main.cpp
)

target_link_libraries(sql_client
    ${Boost_LIBRARIES}
)

# Platform-specific libraries for client
if(WIN32)
    target_link_libraries(sql_client ws2_32 wsock32)
else()
    target_link_libraries(sql_client pthread)
endif()

# Install targets
install(TARGETS sql_server sql_client
    RUNTIME DESTINATION bin
)

# Copy headers for reference
install(FILES
    common/protocol.hpp
    server/sql_server.hpp
    client/sql_client.hpp
    DESTINATION include/asio-sql-service
)
