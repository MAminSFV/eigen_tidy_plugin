#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $0 <format|check>" >&2
  exit 2
}

[[ $# -eq 1 ]] || usage

MODE=$1
case "$MODE" in
  format|check)
    ;;
  *)
    usage
    ;;
esac

SCRIPT_DIR=$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)
CLANG_FORMAT=${CLANG_FORMAT:-clang-format}

if ! command -v "$CLANG_FORMAT" >/dev/null 2>&1; then
  echo "Error: clang-format not found in PATH." >&2
  exit 1
fi

mapfile -d '' FILES < <(find "$REPO_ROOT" \
  \( -path "$REPO_ROOT/build" -o -path "$REPO_ROOT/example_repo/build" -o -path "$REPO_ROOT/.git" \) -prune -o \
  \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' \) -print0)

if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "No source files found for clang-format." >&2
  exit 0
fi

cd "$REPO_ROOT"

if [[ "$MODE" == "format" ]]; then
  "$CLANG_FORMAT" --style=file -i "${FILES[@]}"
else
  "$CLANG_FORMAT" --style=file -n --Werror "${FILES[@]}"
fi
