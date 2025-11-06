#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $0 <check|fix> [build-dir|compile-commands.json]" >&2
  exit 2
}

[[ $# -ge 1 && $# -le 2 ]] || usage
MODE=$1
case "$MODE" in
  check|fix) ;;
  *) usage ;;
esac

SCRIPT_DIR=$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

resolve_path() {
  local input=$1
  if [[ $input == /* ]]; then
    printf '%s\n' "$input"
  else
    printf '%s\n' "$REPO_ROOT/$input"
  fi
}

CLANG_TIDY=${CLANG_TIDY:-}
if [[ -z "$CLANG_TIDY" ]]; then
  if command -v clang-tidy >/dev/null 2>&1; then
    CLANG_TIDY=$(command -v clang-tidy)
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
      CLANG_TIDY=$(printf '%s\n' "${versioned_bins[@]}" | sort -V | tail -n1)
    fi
  fi
fi

if [[ -z "$CLANG_TIDY" || ! -x "$CLANG_TIDY" ]]; then
  echo "Error: clang-tidy executable not found in PATH." >&2
  exit 1
fi

TARGET_BUILD=${2:-build}
TARGET_BUILD=$(resolve_path "$TARGET_BUILD")

if [[ -d "$TARGET_BUILD" ]]; then
  COMPILE_DB="$TARGET_BUILD/compile_commands.json"
else
  COMPILE_DB="$TARGET_BUILD"
fi

if [[ ! -f "$COMPILE_DB" ]]; then
  for candidate in \
    "$REPO_ROOT/build/compile_commands.json" \
    "$REPO_ROOT/example_repo/build/compile_commands.json"; do
    if [[ -f "$candidate" ]]; then
      COMPILE_DB="$candidate"
      break
    fi
  done
fi

if [[ ! -f "$COMPILE_DB" ]]; then
  echo "Error: compile_commands.json not found. Provide a build directory or generate the compilation database." >&2
  exit 1
fi

COMPILE_DB_DIR=$(cd -- "$(dirname "$COMPILE_DB")" && pwd)

mapfile -d '' FILES < <(python3 - <<'PY' "$COMPILE_DB" "$REPO_ROOT"
import json
import os
import sys

compile_db = sys.argv[1]
repo_root = os.path.realpath(sys.argv[2])
with open(compile_db, 'r', encoding='utf-8') as fh:
    entries = json.load(fh)

seen = set()
for entry in entries:
    path = entry.get('file')
    if not path:
        continue
    if not os.path.isabs(path):
        directory = entry.get('directory', os.path.dirname(compile_db))
        path = os.path.normpath(os.path.join(directory, path))
    path = os.path.realpath(path)
    try:
        common = os.path.commonpath([repo_root, path])
    except ValueError:
        continue
    if common != repo_root:
        continue
    if path in seen:
        continue
    seen.add(path)
    sys.stdout.write(path + '\0')
PY
)

if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "No translation units found in $COMPILE_DB." >&2
  exit 0
fi

BASE_ARGS=("$CLANG_TIDY" "-p" "$COMPILE_DB_DIR" "-quiet" "--extra-arg=-Wno-everything")
if [[ "$MODE" == "check" ]]; then
  BASE_ARGS+=("-warnings-as-errors=*")
else
  BASE_ARGS+=("-fix" "-format")
fi

for file in "${FILES[@]}"; do
  "${BASE_ARGS[@]}" "$file"
done
