cmake_minimum_required(VERSION 3.16)
project(clang-tidy-eigen-avoid-auto)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set up LLVM and Clang includes
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
# Add clang-tidy specific headers (usually in LLVM installation)
include_directories("${LLVM_INSTALL_PREFIX}/include")
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Add our include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Create the plugin as a MODULE library
add_library(clang-tidy-eigen MODULE
    src/EigenModule.cpp
    src/AvoidAutoForEigenCheck.cpp
)

# Link against required libraries
target_link_libraries(clang-tidy-eigen
    clangTidy
    clangTidyUtils
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangLex
    clangTooling
    clangSerialization
    clangDriver
    clangParse
    clangSema
    clangEdit
    clangAnalysis
    clangRewrite
)

# Ensure the plugin doesn't have a lib prefix and uses .so extension
set_target_properties(clang-tidy-eigen PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# Optional: Add example project
add_subdirectory(example/mini)
