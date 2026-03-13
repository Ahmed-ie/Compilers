#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/ir_builder/ir_builder"
TEST_DIR="$ROOT/tests/builder_tests"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

pass_count=0
fail_count=0
overall=0

if [ ! -x "$BIN" ]; then
    echo "IR builder binary not found at $BIN"
    echo "Build first with: make"
    exit 1
fi

for src in "$TEST_DIR"/p*.c; do
    base="$(basename "$src" .c)"
    out="/tmp/$base.ll"

    echo "== $src -> $out =="
    if "$BIN" "$src" "$out" && [ -s "$out" ]; then
        echo -e "${GREEN}PASS${NC}"
        pass_count=$((pass_count + 1))
    else
        echo -e "${RED}FAIL${NC}"
        fail_count=$((fail_count + 1))
        overall=1
    fi
done

echo
echo "Summary: $pass_count passed, $fail_count failed"
exit "$overall"
