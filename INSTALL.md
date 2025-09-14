# Installation Guide

This document explains how to install the Eigen Tidy Plugin using the APT repository.

## Install from APT Repository

### Step 1: Add the Repository

Add the APT repository to your system:

```bash
echo 'deb [trusted=yes] https://maminfv.github.io/my_tidy_plugin/ ./' | sudo tee /etc/apt/sources.list.d/eigen-tidy-plugin.list
```

### Step 2: Update APT Cache

```bash
sudo apt update
```

### Step 3: Install the Package

```bash
sudo apt install eigen-tidy-plugin
```

## Using the Plugin

After installation, you can use the plugin with clang-tidy:

```bash
clang-tidy -load /usr/lib/clang-tidy-plugins/eigen_tidy_plugin.so -checks=-*,eigen-avoid-auto your_file.cpp
```

Or add it to your `.clang-tidy` configuration file:

```yaml
Checks: '-*,eigen-avoid-auto'
```

## Building from Source

If you prefer to build from source:

### Prerequisites

- CMake 3.16 or later
- LLVM/Clang development libraries
- C++17 compatible compiler

### Build Steps

```bash
git clone https://github.com/MAminSFV/my_tidy_plugin.git
cd my_tidy_plugin/eigen_tidy_plugin
mkdir build && cd build
cmake ..
make
```

### Install Locally

```bash
sudo make install
```

## Creating Your Own Package

To create a `.deb` package locally:

```bash
cd eigen_tidy_plugin/build
cmake --build . --target package
```

This will create a `.deb` file in the build directory.

## Security Note

The repository is currently configured with `[trusted=yes]` for simplicity. For production use, you should:

1. Set up GPG signing for the repository
2. Remove the `[trusted=yes]` flag
3. Import the GPG public key

## Support

If you encounter issues:

1. Check that LLVM/Clang development packages are installed
2. Verify that the plugin loads correctly with clang-tidy
3. Open an issue on GitHub: https://github.com/MAminSFV/my_tidy_plugin

## Uninstall

To remove the package:

```bash
sudo apt remove eigen-tidy-plugin
sudo rm /etc/apt/sources.list.d/eigen-tidy-plugin.list
sudo apt update
```
