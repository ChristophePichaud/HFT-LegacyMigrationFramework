cmake_minimum_required(VERSION 3.16)
project(LegacyMigrationFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_TESTS "Build unit tests" ON)
option(USE_POSTGRESQL "Enable PostgreSQL backend" OFF)
option(USE_SYBASE "Enable Sybase backend" OFF)

# Add include directory for header-only libraries
include_directories(${CMAKE_SOURCE_DIR}/include)

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

target_include_directories(core PUBLIC src ${CMAKE_SOURCE_DIR}/include)

if (USE_POSTGRESQL)
    add_library(pg_backend STATIC ${PG_BACKEND})
    target_include_directories(pg_backend PUBLIC src ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(pg_backend PUBLIC core PostgreSQL::PostgreSQL)
endif()

if (USE_SYBASE)
    add_library(sybase_backend STATIC ${SYB_BACKEND})
    target_include_directories(sybase_backend PUBLIC src ${CMAKE_SOURCE_DIR}/include ${SYBASE_INCLUDE_DIR})
    target_link_libraries(sybase_backend PUBLIC core ${SYBASE_LIBRARY})
endif()

add_executable(lmf_codegen ${CODEGEN})
target_include_directories(lmf_codegen PRIVATE src ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(lmf_codegen PRIVATE core)

if (BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    file(GLOB TEST_SOURCES tests/*.cpp)

    add_executable(unit_tests ${TEST_SOURCES})
    target_include_directories(unit_tests PRIVATE src tests ${CMAKE_SOURCE_DIR}/include)
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
