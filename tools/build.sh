#!/bin/bash

# Get the root directory of the project
ROOT_DIR=$(git rev-parse --show-toplevel)

# Create and navigate to the build directory
BUILD_DIR="$ROOT_DIR/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake to configure the project
cmake "$ROOT_DIR/eigen_tidy_plugin"

# Build the project
make -j$(nproc)

cd "$ROOT_DIR"
