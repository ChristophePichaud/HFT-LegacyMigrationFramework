cmake_minimum_required(VERSION 3.15)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Test executables
if(WITH_POSTGRESQL)
    add_executable(test_postgresql 
        test_postgresql.cpp
    )
    target_link_libraries(test_postgresql
        hft-legacy-migration
        gtest_main
    )
    add_test(NAME PostgreSQLTests COMMAND test_postgresql)
endif()

if(WITH_SYBASE)
    add_executable(test_sybase 
        test_sybase.cpp
    )
    target_link_libraries(test_sybase
        hft-legacy-migration
        gtest_main
    )
    add_test(NAME SybaseTests COMMAND test_sybase)
endif()

# ORM and reflection tests
add_executable(test_orm
    test_orm.cpp
)
target_link_libraries(test_orm
    hft-legacy-migration
    gtest_main
)
add_test(NAME ORMTests COMMAND test_orm)

# Catalog tests
add_executable(test_catalog
    test_catalog.cpp
)
target_link_libraries(test_catalog
    hft-legacy-migration
    gtest_main
)
add_test(NAME CatalogTests COMMAND test_catalog)
