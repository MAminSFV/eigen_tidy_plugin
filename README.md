# clang-tidy-eigen-avoid-auto

A clang-tidy plugin providing the `eigen-avoid-auto` check that diagnoses usage of `auto` with Eigen types and expressions.

## Overview

This plugin implements a clang-tidy check that warns against using `auto` for Eigen types or expressions, helping developers avoid common pitfalls related to expression templates and temporary objects in Eigen code.

### What it detects:

- ✅ `auto` with Eigen plain objects: `auto m = matrix;`
- ✅ `auto` with Eigen expressions: `auto result = m1 + m2;`
- ✅ `const auto` with Eigen types: `const auto m = matrix;`
- ✅ `auto&` with Eigen types: `auto& ref = matrix;`
- ✅ `decltype(auto)` with Eigen types: `decltype(auto) m = matrix;`

### What it allows:

- ✅ Explicit Eigen types: `Eigen::MatrixXd m = matrix;`
- ✅ `auto` with `.eval()` calls: `auto m = (expr).eval();`
- ✅ `auto` with non-Eigen types: `auto x = 42;`
- ✅ Range-based for loops (configurable): `for (auto& m : matrices)`

## The Problem

As documented in [Eigen's pitfalls page](https://libeigen.gitlab.io/eigen/docs-nightly/TopicPitfalls.html), using `auto` with Eigen can lead to performance issues and bugs due to expression templates:

```cpp
// Problematic - stores expression template, not the result
auto m = matrix1 + matrix2;  // Warning: eigen-avoid-auto

// Better - explicit type forces evaluation
Eigen::MatrixXd m = matrix1 + matrix2;  // OK

// Or use .eval() to force evaluation
auto m = (matrix1 + matrix2).eval();    // OK
```

## Building

### Prerequisites

- CMake 3.16+
- LLVM/Clang 15+ (17+ recommended, 18+ tested)
- C++17 compiler

#### Installing LLVM/Clang on Ubuntu

```bash
# Install LLVM 18 (recommended)
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 18

# Install development packages
sudo apt-get install -y \
    llvm-18-dev \
    clang-18 \
    clang-tidy-18 \
    libclang-18-dev \
    cmake \
    build-essential
```

### Build Steps

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/llvm
make -j$(nproc)
```

### Using with clang-tidy

```bash
# Load the plugin and run the check
clang-tidy --load ./build/clang-tidy-eigen.so --checks='-*,eigen-avoid-auto' file.cpp

# With C++ standard and include paths
clang-tidy --load ./build/clang-tidy-eigen.so --checks='-*,eigen-avoid-auto' \
    file.cpp -- -std=c++17 -I/usr/include/eigen3

# Configure options via .clang-tidy file (recommended)
clang-tidy --load ./build/clang-tidy-eigen.so --checks='-*,eigen-avoid-auto' file.cpp

# Or configure inline (less readable)
clang-tidy --load ./build/clang-tidy-eigen.so --checks='-*,eigen-avoid-auto' \
    --config="{CheckOptions: [{key: eigen-avoid-auto.AllowInRangeFor, value: true}]}" \
    file.cpp
```

## Check Configuration

The `eigen-avoid-auto` check supports several options:

- **AllowInRangeFor** (bool, default: false): Allow `auto` in range-based for loops
- **OnlyExpressions** (bool, default: false): Only warn for expression templates, not plain Eigen objects
- **BanDecltypeAuto** (bool, default: true): Also warn for `decltype(auto)`

### Example .clang-tidy Configuration

```yaml
Checks: 'eigen-avoid-auto'
CheckOptions:
  - key: eigen-avoid-auto.AllowInRangeFor
    value: true
  - key: eigen-avoid-auto.OnlyExpressions
    value: false
  - key: eigen-avoid-auto.BanDecltypeAuto
    value: true
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

### Demo Project

See the `example/mini/` directory for a complete sample project demonstrating the check.

## Testing

The plugin can be tested using the provided test files:

```bash
# Test with the basic test cases
clang-tidy --load ./build/clang-tidy-eigen.so --checks='-*,eigen-avoid-auto' \
    test_cases.cpp -- -std=c++17

# Test with the example project (using mock Eigen)
cd example/mini
clang-tidy --load ../../build/clang-tidy-eigen.so --checks='-*,eigen-avoid-auto' \
    main.cpp -- -std=c++17 -DMOCK_EIGEN

# Test with real Eigen (if installed)
clang-tidy --load ../../build/clang-tidy-eigen.so --checks='-*,eigen-avoid-auto' \
    main.cpp -- -std=c++17 -I/usr/include/eigen3

# Run the automated test script
./tests/run.sh
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
clang-tidy --load ./build/clang-tidy-eigen.so --checks='-*,eigen-avoid-auto' test.cpp -- -std=c++17
rm test.cpp

# Check that the .so file was built
ls -la build/clang-tidy-eigen.so
file build/clang-tidy-eigen.so  # Should show it's a shared library
```

## License

MIT License - see LICENSE file for details.
