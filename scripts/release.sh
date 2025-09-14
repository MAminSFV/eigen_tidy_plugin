#!/bin/bash

# Release build script for Eigen Tidy Plugin
# This script handles clean build, packaging, and APT repository generation

set -euo pipefail  # Exit on error, undefined vars, pipe failures

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Default values
PROJECT_VERSION="${PROJECT_VERSION:-0.1.0}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="build"
REPO_DIR="repo"
CLEAN_BUILD="${CLEAN_BUILD:-true}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PLUGIN_DIR="$PROJECT_ROOT/eigen_tidy_plugin"

# Function to show usage
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

OPTIONS:
    -v, --version VERSION    Set the project version (default: $PROJECT_VERSION)
    -t, --type BUILD_TYPE    Set build type: Release|Debug (default: $BUILD_TYPE)
    -d, --build-dir DIR      Set build directory (default: $BUILD_DIR)
    -r, --repo-dir DIR       Set repository output directory (default: $REPO_DIR)
    -k, --keep-build         Keep existing build directory (don't clean)
    -h, --help               Show this help message

ENVIRONMENT VARIABLES:
    PROJECT_VERSION          Project version (overridden by -v)
    BUILD_TYPE              Build type (overridden by -t)
    CLEAN_BUILD             Clean build directory (true/false)

EXAMPLES:
    $0                                    # Build with defaults
    $0 -v 1.2.3                         # Build version 1.2.3
    $0 -v 1.2.3 -t Debug                # Build debug version 1.2.3
    $0 -k                                # Keep existing build directory
    CLEAN_BUILD=false $0                 # Don't clean build (env var)

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--version)
            PROJECT_VERSION="$2"
            shift 2
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -d|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -r|--repo-dir)
            REPO_DIR="$2"
            shift 2
            ;;
        -k|--keep-build)
            CLEAN_BUILD="false"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Validate version format (should be semver-like)
if [[ ! "$PROJECT_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-.*)?$ ]]; then
    log_warning "Version '$PROJECT_VERSION' doesn't follow semantic versioning format (x.y.z)"
fi

log_info "Starting release build process..."
log_info "Project Version: $PROJECT_VERSION"
log_info "Build Type: $BUILD_TYPE"
log_info "Build Directory: $BUILD_DIR"
log_info "Repository Directory: $REPO_DIR"
log_info "Clean Build: $CLEAN_BUILD"

# Change to plugin directory
cd "$PLUGIN_DIR"
log_info "Working in: $(pwd)"

# Clean build directory if requested
if [[ "$CLEAN_BUILD" == "true" && -d "$BUILD_DIR" ]]; then
    log_info "Cleaning existing build directory..."
    rm -rf "$BUILD_DIR"
fi

# Export version for CMake
export PROJECT_VERSION

# Configure the project
log_info "Configuring CMake project..."
cmake -B "$BUILD_DIR" -S . \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_TESTING=ON

# Build the project
log_info "Building project..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

# Run tests if available
if [[ -f "$BUILD_DIR/test_cases" ]]; then
    log_info "Running tests..."
    cd "$BUILD_DIR"
    ctest --output-on-failure || {
        log_warning "Some tests failed, but continuing with packaging..."
    }
    cd ..
fi

# Create the package
log_info "Creating Debian package..."
cd "$BUILD_DIR"
cpack

# Find the generated .deb file
DEB_FILE=$(find . -name "*.deb" -type f | head -n 1)
if [[ -z "$DEB_FILE" ]]; then
    log_error "No .deb file found after packaging!"
    exit 1
fi

DEB_FILE=$(basename "$DEB_FILE")
log_success "Generated package: $DEB_FILE"

# Create APT repository
log_info "Creating APT repository structure..."
cd "$PLUGIN_DIR"
rm -rf "$REPO_DIR"
mkdir -p "$REPO_DIR"

# Copy .deb file to repo directory
cp "$BUILD_DIR"/*.deb "$REPO_DIR/"

# Generate Packages.gz
cd "$REPO_DIR"
log_info "Generating APT metadata..."
dpkg-scanpackages . /dev/null | gzip -9c > Packages.gz

# Create Release file
log_info "Creating Release file..."
cat > Release << EOF
Origin: MAminSFV
Label: Eigen Tidy Plugin Repository
Suite: stable
Codename: stable
Version: 1.0
Architectures: amd64
Components: main
Description: APT repository for Eigen Tidy Plugin
Date: $(date -Ru)
EOF

# Generate file checksums for Release file
log_info "Generating checksums..."
{
    echo "MD5Sum:"
    md5sum Packages.gz | sed 's/^/ /'
    echo "SHA1:"
    sha1sum Packages.gz | sed 's/^/ /'
    echo "SHA256:"
    sha256sum Packages.gz | sed 's/^/ /'
} >> Release

# Show summary
cd "$PLUGIN_DIR"
log_success "Release build completed successfully!"
echo
log_info "Generated files:"
echo "  - Debian package: $BUILD_DIR/$DEB_FILE"
echo "  - APT repository: $REPO_DIR/"
echo "    ├── $DEB_FILE"
echo "    ├── Packages.gz"
echo "    └── Release"

# Show package info
log_info "Package information:"
dpkg --info "$BUILD_DIR/$DEB_FILE" | grep -E "(Package|Version|Architecture|Description)" | sed 's/^/  /'

echo
log_success "Release build process completed!"
log_info "You can now:"
echo "  - Install locally: sudo dpkg -i $BUILD_DIR/$DEB_FILE"
echo "  - Test repository: dpkg-scanpackages $REPO_DIR"
echo "  - Deploy to GitHub Pages: copy $REPO_DIR/* to gh-pages branch"
