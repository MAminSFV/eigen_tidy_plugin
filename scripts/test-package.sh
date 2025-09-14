#!/bin/bash

# Simple test script to verify the generated .deb package works
# This script can be used in CI or locally to test the package

set -euo pipefail

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

# Find the latest .deb package
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PLUGIN_DIR="$PROJECT_ROOT/eigen_tidy_plugin"

DEB_FILE=$(find "$PLUGIN_DIR" -name "*.deb" -type f | sort | tail -n 1)

if [[ -z "$DEB_FILE" ]]; then
    log_error "No .deb package found. Run release script first."
    exit 1
fi

log_info "Testing package: $(basename "$DEB_FILE")"

# Test package info
log_info "Checking package metadata..."
if dpkg --info "$DEB_FILE" | grep -q "eigen_tidy_plugin"; then
    log_success "Package metadata is valid"
else
    log_error "Package metadata is invalid"
    exit 1
fi

# Test package contents
log_info "Checking package contents..."
if dpkg --contents "$DEB_FILE" | grep -q "eigen_tidy_plugin.so"; then
    log_success "Plugin library is included"
else
    log_error "Plugin library is missing"
    exit 1
fi

if dpkg --contents "$DEB_FILE" | grep -q "avoid_auto_check.hpp"; then
    log_success "Header file is included"
else
    log_error "Header file is missing"
    exit 1
fi

# Test installation (if running as root or with sudo)
if [[ $EUID -eq 0 ]] || sudo -n true 2>/dev/null; then
    log_info "Testing installation..."

    # Install package
    sudo dpkg -i "$DEB_FILE" 2>/dev/null || {
        log_error "Package installation failed"
        exit 1
    }
    log_success "Package installed successfully"

    # Check files are in place
    if [[ -f "/usr/lib/clang-tidy-plugins/eigen_tidy_plugin.so" ]]; then
        log_success "Plugin library installed correctly"
    else
        log_error "Plugin library not found after installation"
        exit 1
    fi

    if [[ -f "/usr/include/eigen-tidy-plugin/avoid_auto_check.hpp" ]]; then
        log_success "Header file installed correctly"
    else
        log_error "Header file not found after installation"
        exit 1
    fi

    # Test plugin loading (if clang-tidy is available)
    if command -v clang-tidy >/dev/null 2>&1; then
        log_info "Testing plugin loading with clang-tidy..."
        if clang-tidy --load /usr/lib/clang-tidy-plugins/eigen_tidy_plugin.so --help >/dev/null 2>&1; then
            log_success "Plugin loads successfully with clang-tidy"
        else
            log_error "Plugin failed to load with clang-tidy"
            exit 1
        fi
    else
        log_info "clang-tidy not available, skipping plugin load test"
    fi

    # Remove package
    log_info "Removing package..."
    sudo dpkg -r eigen_tidy_plugin 2>/dev/null || {
        log_error "Package removal failed"
        exit 1
    }
    log_success "Package removed successfully"

else
    log_info "Not running as root, skipping installation test"
    log_info "To test installation, run: sudo $0"
fi

log_success "All tests passed!"
echo
log_info "Package is ready for distribution!"
