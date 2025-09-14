# Eigen Tidy Plugin

A clang-tidy plugin for checking Eigen code.

TODOs before making repo public
- [ ] Add docker for dev and installation testing
- [ ] Add dev container setup for development example usage
- [ ] Re-write the workflows with the docker setup (add cd?)
- [ ] Go through the CPack data
- [ ] Go through tests and add more
- [ ] Add for external review
- [ ] Add a copilot instructions file
- [ ] Clean up the readmes and make them informative and concise


## Overview

This plugin provides a clang-tidy check `eigen-avoid-auto` to avoid using `auto` with Eigen types.

## Building

### Prerequisites

- CMake (>= 3.16)
- LLVM/Clang development packages
- For testing:
  - `libeigen3-dev` - Eigen3 development headers
  - `libgtest-dev` - Google Test framework

#### Installing Dependencies on Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y cmake clang-tidy llvm-dev libclang-dev libeigen3-dev libgtest-dev
```

### Build Steps

```bash
./scripts/build_and_test.sh
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
clang-tidy --load ./build/eigen_tidy_plugin.so --checks='-*,eigen-avoid-auto' \
    tests/test_cases.cpp -- -std=c++17

# Create a simple test file to verify the plugin works
clang-tidy --load ../../build/eigen_tidy_plugin.so --checks='-*,eigen-avoid-auto' \
    main.cpp -- -std=c++17 -DMOCK_EIGEN

# Test with real Eigen (if installed)
clang-tidy --load ../../build/eigen_tidy_plugin.so --checks='-*,eigen-avoid-auto' \
    main.cpp -- -std=c++17 -I/usr/include/eigen3

# The build_and_test.sh script runs all automated tests
./scripts/build_and_test.sh
```

## Troubleshooting

### Plugin Loading Issues

If you get "symbol lookup error" when loading the plugin:
- Ensure you're using the same LLVM/Clang version for building and running
- Make sure all required development packages are installed
- Try building with the exact same compiler used to build your clang-tidy

### Build Issues

If CMake can't find LLVM/Clang:
```bash
# Find your LLVM installation
find /usr -name "LLVMConfig.cmake" 2>/dev/null
find /usr -name "ClangConfig.cmake" 2>/dev/null

# Set the paths explicitly
cmake .. -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm \
         -DClang_DIR=/usr/lib/llvm-18/lib/cmake/clang
```

### Check Not Found

If clang-tidy can't find the `eigen-avoid-auto` check:
```bash
# Verify the plugin loads correctly by testing it
echo "namespace Eigen { template<class T, int R, int C> class Matrix {}; }" > test.cpp
echo "void test() { using namespace Eigen; Matrix<double,-1,-1> m; auto x = m; }" >> test.cpp
clang-tidy --load ./build/eigen_tidy_plugin.so --checks='-*,eigen-avoid-auto' test.cpp -- -std=c++17
rm test.cpp

# Check that the .so file was built
ls -la build/eigen_tidy_plugin.so
file build/eigen_tidy_plugin.so  # Should show it's a shared library
```
