# Eigen Tidy Plugin
A clang-tidy plugin for checking Eigen code.

## Overview
This plugin provides a clang-tidy check `eigen-avoid-auto` to avoid using `auto` with Eigen types.

## Building

### Prerequisites

- CMake (>= 3.16)
- LLVM/Clang development packages
- For testing:
  - `libeigen3-dev` - Eigen3 development headers

#### Installing Dependencies on Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y cmake clang-tidy llvm-dev libclang-dev libeigen3-dev
```

### Build Steps

#### Native Build
```bash
./tools/build.sh
```

#### Docker Build (for consistent environments)
```bash
# Build the container and run tests
docker build -f tools/Dockerfile --target builder -t eigen-plugin .
docker run --rm -v $(pwd):/workspace -w /workspace eigen-plugin ./tools/build.sh
```

## Running

The build script automatically runs the tests. To manually test the plugin:

```bash
# Test the plugin on the provided test cases
cd eigen_tidy_plugin
clang-tidy --load ./build/eigen_tidy_plugin.so \
  --checks='-*,eigen-avoid-auto' \
  tests/test_cases.cpp -- -std=c++17
```


## Examples

### Code Examples

```cpp
#include <Eigen/Dense>

void example() {
    Eigen::MatrixXd m1(3, 3), m2(3, 3);

    // These will trigger warnings:
    auto bad1 = m1;                    // eigen-avoid-auto warning
    auto bad2 = m1 + m2;               // eigen-avoid-auto warning
    const auto bad3 = m1;              // eigen-avoid-auto warning
    auto& bad4 = m1;                   // eigen-avoid-auto warning
    decltype(auto) bad5 = m1;          // eigen-avoid-auto warning

    // These are OK:
    Eigen::MatrixXd good1 = m1;        // Explicit type
    Eigen::MatrixXd good2 = m1 + m2;   // Explicit type forces evaluation
    auto good3 = m1.eval();            // Explicit .eval() call
    auto good4 = 42;                   // Not an Eigen type
}
```

### Expected Output

```
warning: do not use 'auto' for Eigen types or expressions; declare an explicit
Eigen type or assign the whole expression to a concrete type (e.g., (expr).eval()
into Eigen::Matrix<>). See Eigen pitfalls:
https://libeigen.gitlab.io/eigen/docs-nightly/TopicPitfalls.html [eigen-avoid-auto]
     auto bad1 = m1;
     ~~~~ ^
```

## Testing

The plugin can be tested using the provided test files:

```bash
# Test with the basic test cases
cd eigen_tidy_plugin
./tools/test_example_repo_local_build
```
