# clang-tidy-eigen-avoid-auto

A clang-tidy plugin providing the `eigen-avoid-auto` check that diagnoses usage of `auto` with Eigen types and expressions.

## Overview

This plugin implements a clang-tidy check that warns against using `auto` for Eigen types or expressions, helping developers avoid common pitfalls related to expression templates and temporary objects in Eigen code.

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
- LLVM/Clang 15+ (17+ recommended)
- C++17 compiler

### Build Steps

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/llvm
make -j$(nproc)
```

### Using with clang-tidy

```bash
# Load the plugin and run the check
clang-tidy -load ./clang-tidy-eigen.so -checks='-*,eigen-avoid-auto' file.cpp

# Configure options
clang-tidy -load ./clang-tidy-eigen.so -checks='-*,eigen-avoid-auto' \
    -config="{CheckOptions: [{key: eigen-avoid-auto.AllowInRangeFor, value: true}]}" \
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

See the `example/mini/` directory for a sample project demonstrating the check.

## Testing

```bash
cd tests
./run.sh
```

## License

MIT License - see LICENSE file for details.
