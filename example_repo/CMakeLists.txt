cmake_minimum_required(VERSION 3.16)

project(example_repo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

add_executable(example_repo src/example.cpp)
target_compile_definitions(example_repo PRIVATE TEST_MAIN)
target_link_libraries(example_repo Eigen3::Eigen)
