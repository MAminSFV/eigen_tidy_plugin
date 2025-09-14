#!/bin/bash

set -e

cd eigen_tidy_plugin

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to the build directory
cd build

# Run CMake to configure the project
cmake .. -DBUILD_TESTING=ON

# Build the project
make -j$(nproc)

# Run tests
ctest
