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

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set up LLVM includes
include_directories(${LLVM_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})

# Create the plugin as a MODULE library
add_library(eigen_tidy_plugin MODULE
    avoid_auto_check.cpp
    eigen_module.cpp
)

# Determine LLVM library paths
string(REGEX MATCH "^[0-9]+" LLVM_VERSION_MAJOR "${LLVM_PACKAGE_VERSION}")
string(REGEX MATCH "^[0-9]+\\.[0-9]+" LLVM_VERSION_MAJOR_MINOR "${LLVM_PACKAGE_VERSION}")
set(LLVM_LIB_DIR "/usr/lib/llvm-${LLVM_VERSION_MAJOR}/lib")

# Link against required Clang and LLVM libraries
target_link_libraries(eigen_tidy_plugin PRIVATE
    ${LLVM_LIB_DIR}/libclang-cpp.so.${LLVM_VERSION_MAJOR_MINOR}
    ${LLVM_LIB_DIR}/libclangAST.a
    ${LLVM_LIB_DIR}/libclangAnalysis.a
    ${LLVM_LIB_DIR}/libclangBasic.a
    ${LLVM_LIB_DIR}/libclangLex.a
    ${LLVM_LIB_DIR}/libclangSema.a
    LLVMCore
    LLVMSupport
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

    # Create a simple test that compiles the test cases
    add_executable(test_cases tests/test_cases.cpp)
    target_compile_definitions(test_cases PRIVATE -DTEST_MAIN)
    add_test(NAME compile_test_cases COMMAND test_cases)
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
