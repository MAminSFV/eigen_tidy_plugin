cmake_minimum_required(VERSION 3.16)

project(example_repo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(example_repo src/example.cpp)
target_compile_definitions(example_repo PRIVATE TEST_MAIN)
