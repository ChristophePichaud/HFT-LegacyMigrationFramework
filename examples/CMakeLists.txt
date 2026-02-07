cmake_minimum_required(VERSION 3.15)

# Example: Basic usage
add_executable(example_basic basic_usage.cpp)
target_link_libraries(example_basic hft-legacy-migration)

# Example: ORM usage
add_executable(example_orm orm_usage.cpp)
target_link_libraries(example_orm hft-legacy-migration)

# Example: Code generation
add_executable(example_codegen codegen_usage.cpp)
target_link_libraries(example_codegen hft-legacy-migration)
