cmake_minimum_required(VERSION 3.16)
project(eigen_tidy_plugin)

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

# Create the plugin as a MODULE library
add_library(eigen_tidy_plugin MODULE
    avoid_auto_check.cpp
    eigen_module.cpp
)

# Link against required libraries
target_link_libraries(eigen_tidy_plugin
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
set_target_properties(eigen_tidy_plugin PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

if(BUILD_TESTING)
    enable_testing()

    # Include eigen as a dependency for test cases
    find_package(Eigen3 REQUIRED)

    # Create a simple test that compiles the test cases
    # Add a main function to make it executable
    add_executable(test_cases tests/test_cases.cpp)

    target_link_libraries(test_cases PRIVATE
        Eigen3::Eigen)

    # Add a main function via compiler definition
    target_compile_definitions(test_cases PRIVATE -DTEST_MAIN)

    # Add a test that just ensures the test cases compile and run
    add_test(NAME compile_test_cases COMMAND test_cases)

    # TODO: Add integration tests that run clang-tidy with the plugin
    # This would require a separate test script that invokes clang-tidy
    # with the plugin on the test files
endif()
