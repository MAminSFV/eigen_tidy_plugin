#!/bin/bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Running clang-tidy-eigen-avoid-auto tests...${NC}"

# Check if we're in the right directory or adjust path
if [[ -f "CMakeLists.txt" ]]; then
    # We're in the project root
    BUILD_DIR="build"
elif [[ -f "../CMakeLists.txt" ]]; then
    # We're in the tests directory
    BUILD_DIR="../build"
else
    echo -e "${RED}Error: Cannot find CMakeLists.txt. Please run from project root or tests/ directory${NC}"
    exit 1
fi

# Make sure the plugin is built
if [[ ! -f "$BUILD_DIR/clang-tidy-eigen.so" ]]; then
    echo -e "${YELLOW}Plugin not found. Building...${NC}"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake ..
    make -j$(nproc)
    cd - > /dev/null
fi

# Check if plugin exists now
if [[ ! -f "$BUILD_DIR/clang-tidy-eigen.so" ]]; then
    echo -e "${RED}Error: Failed to build plugin${NC}"
    exit 1
fi

echo -e "${GREEN}Plugin found at $BUILD_DIR/clang-tidy-eigen.so${NC}"

# Test file
TEST_FILE="test_cases.cpp"

# Create test file with various cases
cat > "$TEST_FILE" << 'EOF'
// Mock Eigen for testing
namespace Eigen {
    template<typename T, int Rows, int Cols>
    class Matrix {
    public:
        Matrix() = default;
        Matrix operator+(const Matrix& other) const { return Matrix{}; }
        Matrix eval() const { return *this; }
    };

    template<typename Derived>
    class PlainObjectBase {};

    using MatrixXd = Matrix<double, -1, -1>;
    using Vector3d = Matrix<double, 3, 1>;
}

void test_function() {
    using namespace Eigen;

    MatrixXd m1, m2;
    Vector3d v1, v2;

    // Should trigger warnings
    auto bad1 = m1;              // eigen-avoid-auto
    auto bad2 = m1 + m2;         // eigen-avoid-auto
    const auto bad3 = v1;        // eigen-avoid-auto
    auto& bad4 = m1;             // eigen-avoid-auto
    decltype(auto) bad5 = m1;    // eigen-avoid-auto (if BanDecltypeAuto=true)

    // Should be OK
    MatrixXd good1 = m1;         // OK
    MatrixXd good2 = m1 + m2;    // OK
    auto good3 = m1.eval();      // OK
    auto good4 = 42;             // OK - not Eigen
}
EOF

# Function to run clang-tidy with specific config
run_test() {
    local config="$1"
    local description="$2"
    local expected_warnings="$3"

    echo -e "\n${YELLOW}Test: $description${NC}"
    echo "Config: $config"

    # Run clang-tidy and count warnings
    local output
    output=$(clang-tidy \
        -load "$BUILD_DIR/clang-tidy-eigen.so" \
        -checks='-*,eigen-avoid-auto' \
        -config="$config" \
        -header-filter='.*' \
        "$TEST_FILE" \
        -- -std=c++17 2>&1 || true)

    local warning_count
    warning_count=$(echo "$output" | grep -c "eigen-avoid-auto" || true)

    echo "Expected warnings: $expected_warnings"
    echo "Actual warnings: $warning_count"

    if [[ "$warning_count" -eq "$expected_warnings" ]]; then
        echo -e "${GREEN}✓ PASS${NC}"
    else
        echo -e "${RED}✗ FAIL${NC}"
        echo "Output:"
        echo "$output"
        return 1
    fi
}

# Test cases
echo -e "\n${YELLOW}Running test cases...${NC}"

# Default configuration
run_test "{}" "Default configuration" 5

# Allow in range-for
run_test "{CheckOptions: [{key: eigen-avoid-auto.AllowInRangeFor, value: true}]}" "Allow in range-for" 5

# Only expressions (this would require more sophisticated test cases to distinguish)
run_test "{CheckOptions: [{key: eigen-avoid-auto.OnlyExpressions, value: true}]}" "Only expressions" 5

# Disable decltype(auto) check
run_test "{CheckOptions: [{key: eigen-avoid-auto.BanDecltypeAuto, value: false}]}" "Disable decltype(auto)" 4

echo -e "\n${YELLOW}Testing example project...${NC}"

# Test the example project
cd ../example/mini
if command -v clang-tidy >/dev/null 2>&1; then
    echo "Running clang-tidy on example project..."
    clang-tidy \
        -load "../../build/clang-tidy-eigen.so" \
        -checks='-*,eigen-avoid-auto' \
        -header-filter='.*' \
        main.cpp \
        -- -std=c++17 -DMOCK_EIGEN || true
else
    echo -e "${YELLOW}clang-tidy not found in PATH, skipping example test${NC}"
fi

cd - > /dev/null

# Cleanup
rm -f "$TEST_FILE"

echo -e "\n${GREEN}All tests completed!${NC}"
echo -e "${YELLOW}Note: Some tests may show expected warnings - this is normal.${NC}"
