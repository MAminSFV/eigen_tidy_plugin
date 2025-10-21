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

# Create the plugin as a MODULE library
add_library(eigen_tidy_plugin MODULE
    avoid_auto_check.cpp
    eigen_module.cpp
)

# Determine LLVM version and paths
string(REGEX MATCH "^[0-9]+\\.[0-9]+" LLVM_VERSION_MAJOR_MINOR "${LLVM_PACKAGE_VERSION}")
set(LLVM_LIB_DIR "/usr/lib/llvm-${LLVM_VERSION_MAJOR}/lib")

# Link against required Clang and LLVM libraries
find_library(CLANG_CPP_LIB
    NAMES
        clang-cpp
        libclang-cpp.so.${LLVM_VERSION_MAJOR}
        libclang-cpp.so
        clang-cpp.${LLVM_VERSION_MAJOR}
    PATHS
        ${LLVM_LIB_DIR}
        /usr/lib/x86_64-linux-gnu
        /usr/lib
    NO_DEFAULT_PATH
)

target_link_libraries(eigen_tidy_plugin PRIVATE
    ${CLANG_CPP_LIB}
    LLVMCore
)

# Add library search paths
target_link_directories(eigen_tidy_plugin PRIVATE ${LLVM_LIB_DIR})

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
