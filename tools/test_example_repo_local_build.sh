#!/bin/bash
set -euo pipefail

ROOT_DIR=$(git rev-parse --show-toplevel)

"$ROOT_DIR/tools/build.sh"

PLUGIN_PATH=""
for candidate in \
    "$ROOT_DIR/build/eigen_tidy_plugin.so" \
    "$ROOT_DIR/build/eigen_tidy_plugin/eigen_tidy_plugin.so" \
    "$ROOT_DIR/build/libeigen_tidy_plugin.so"; do
    if [[ -z "$PLUGIN_PATH" && -f "$candidate" ]]; then
        PLUGIN_PATH="$candidate"
    fi
done

if [[ -z "$PLUGIN_PATH" ]]; then
    echo "Could not find built clang-tidy plugin in $ROOT_DIR/build" >&2
    exit 1
fi

EXAMPLE_DIR="$ROOT_DIR/example_repo"
EXAMPLE_BUILD_DIR="$EXAMPLE_DIR/build"

cmake -S "$EXAMPLE_DIR" -B "$EXAMPLE_BUILD_DIR"
cmake --build "$EXAMPLE_BUILD_DIR" --target example_repo -- -j"$(nproc)"

CLANG_TIDY_BIN=${CLANG_TIDY:-clang-tidy}
if ! command -v "$CLANG_TIDY_BIN" >/dev/null 2>&1; then
    echo "clang-tidy executable '$CLANG_TIDY_BIN' not found in PATH" >&2
    exit 1
fi

"$CLANG_TIDY_BIN" \
    -p "$EXAMPLE_BUILD_DIR" \
    -load "$PLUGIN_PATH" \
    -checks=-*,eigen-avoid-auto \
    "$EXAMPLE_DIR/src/example.cpp"
