cmake_minimum_required(VERSION 3.15)

# Example: Basic usage

add_executable(example_basic basic_usage.cpp)
target_link_libraries(example_basic hft-legacy-migration-static)

# Example: ORM usage

add_executable(example_orm orm_usage.cpp)
target_link_libraries(example_orm hft-legacy-migration-static)

# Example: Code generation

add_executable(example_codegen codegen_usage.cpp)
target_link_libraries(example_codegen hft-legacy-migration-static)

# HFT-Demo: Comprehensive demo application
add_executable(HFT-Demo HFT-Demo.cpp)
target_link_libraries(HFT-Demo hft-legacy-migration-static)
target_include_directories(HFT-Demo PRIVATE ${CMAKE_SOURCE_DIR}/include)
