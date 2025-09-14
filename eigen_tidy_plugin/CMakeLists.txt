cmake_minimum_required(VERSION 3.16)

# Set project version from environment variable or use default
if(DEFINED ENV{PROJECT_VERSION})
    set(PROJECT_VERSION $ENV{PROJECT_VERSION})
else()
    set(PROJECT_VERSION 0.1.0) # default fallback version
endif()

# Extract numeric part for CMake project VERSION (remove any suffix after -)
string(REGEX REPLACE "^([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" PROJECT_SEMVER "${PROJECT_VERSION}")

project(eigen_tidy_plugin VERSION ${PROJECT_SEMVER})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# Derive LLVM installation prefix from LLVM_DIR (…/lib/cmake/llvm)
get_filename_component(LLVM_CMAKE_DIR "${LLVM_DIR}" DIRECTORY)
get_filename_component(LLVM_PREFIX "${LLVM_CMAKE_DIR}" DIRECTORY)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set up LLVM and Clang includes
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${Clang_INCLUDE_DIRS})

# Add clang/clang-tidy headers (under LLVM installation include) as fallback
include_directories("${LLVM_PREFIX}/include")
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Create the plugin as a MODULE library
add_library(eigen_tidy_plugin MODULE
    avoid_auto_check.cpp
    eigen_module.cpp
)

# Ensure the plugin doesn't have a lib prefix and uses .so extension
set_target_properties(eigen_tidy_plugin PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# Install rules for packaging
install(TARGETS eigen_tidy_plugin
    LIBRARY DESTINATION lib/clang-tidy-plugins
    RUNTIME DESTINATION lib/clang-tidy-plugins
)

# Install header files
install(FILES avoid_auto_check.hpp
    DESTINATION include/eigen-tidy-plugin
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

# CPack configuration for Debian packaging
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MAminSFV <mamin@example.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Eigen Tidy Plugin - A clang-tidy plugin for Eigen-specific checks")
set(CPACK_PACKAGE_CONTACT "mamin@example.com")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_PACKAGE_DESCRIPTION "A clang-tidy plugin that provides Eigen-specific linting checks, including avoiding auto declarations in Eigen expressions.")

include(CPack)
