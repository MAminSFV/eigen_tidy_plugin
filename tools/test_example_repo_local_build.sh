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

CLANG_TIDY_BIN=${CLANG_TIDY:-}
if [[ -z "$CLANG_TIDY_BIN" ]]; then
    if command -v clang-tidy >/dev/null 2>&1; then
        CLANG_TIDY_BIN=$(command -v clang-tidy)
    else
        shopt -s nullglob
        IFS=: read -r -a path_dirs <<< "$PATH"
        versioned_bins=()
        for dir in "${path_dirs[@]}"; do
            for candidate in "$dir"/clang-tidy-[0-9]*; do
                [[ -x "$candidate" ]] && versioned_bins+=("$candidate")
            done
        done
        shopt -u nullglob
        if [[ ${#versioned_bins[@]} -gt 0 ]]; then
            CLANG_TIDY_BIN=$(printf '%s\n' "${versioned_bins[@]}" | sort -V | tail -n1)
        fi
    fi
fi

if [[ -z "$CLANG_TIDY_BIN" || ! -x "$CLANG_TIDY_BIN" ]]; then
    echo "clang-tidy executable not found in PATH" >&2
    exit 1
fi

"$CLANG_TIDY_BIN" \
    -p "$EXAMPLE_BUILD_DIR" \
    -load "$PLUGIN_PATH" \
    -checks=-*,eigen-avoid-auto \
    "$EXAMPLE_DIR/src/example.cpp"
