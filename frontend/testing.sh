#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/frontend/parser"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

mode="${1:-}"

if [ ! -x "$BIN" ]; then
    echo "Parser binary not found at $BIN"
    echo "Build first with: make"
    exit 1
fi

case "$mode" in
    parser)
        test_dir="$ROOT/tests/parser"
        ;;
    semantic)
        test_dir="$ROOT/tests/semantic"
        ;;
    *)
        echo "Usage: ./testing.sh [parser|semantic]"
        exit 1
        ;;
esac

pass_count=0
fail_count=0
overall=0

for src in "$test_dir"/p*.c; do
    base="$(basename "$src" .c)"
    expected=0

    if [[ "$base" == *_bad ]]; then
        expected=1
    fi

    echo "== $src =="

    if output="$("$BIN" "$src" 2>&1)"; then
        status=0
    else
        status=$?
    fi

    printf '%s\n' "$output"

    if [ "$mode" = "parser" ]; then
        if [ "$expected" -eq 0 ] && [ "$status" -eq 0 ]; then
            echo -e "${GREEN}PASS${NC}"
            pass_count=$((pass_count + 1))
        elif [ "$expected" -eq 1 ] && [ "$status" -eq 2 ]; then
            echo -e "${GREEN}PASS${NC}"
            pass_count=$((pass_count + 1))
        else
            echo -e "${RED}FAIL${NC}"
            fail_count=$((fail_count + 1))
            overall=1
        fi
    else
        if [ "$expected" -eq 0 ] && [ "$status" -eq 0 ]; then
            echo -e "${GREEN}PASS${NC}"
            pass_count=$((pass_count + 1))
        elif [ "$expected" -eq 1 ] && [ "$status" -eq 3 ]; then
            echo -e "${GREEN}PASS${NC}"
            pass_count=$((pass_count + 1))
        else
            echo -e "${RED}FAIL${NC}"
            fail_count=$((fail_count + 1))
            overall=1
        fi
    fi

    echo
done

echo "Summary: $pass_count passed, $fail_count failed"
exit "$overall"
