cmake_minimum_required(VERSION 3.16)
project(example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example main.cpp)

# Add the path to the built plugin to the CMAKE_CXX_FLAGS
set(CMAKE_CXX_CLANG_TIDY
    clang-tidy
    -load=${CMAKE_BINARY_DIR}/lib/clang-tidy-eigen.so
    -checks=-*,eigen-*
)
