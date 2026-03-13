#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BACKEND_BIN="$ROOT/backend/backend"
TEST_DIR="$ROOT/tests/assembly_gen_tests"
RUNTIME_MAIN="$TEST_DIR/main.c"
TMP_DIR="${TMPDIR:-/tmp}/minic_backend_tests"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

mkdir -p "$TMP_DIR"

if ! command -v clang >/dev/null 2>&1; then
    echo "clang not found"
    exit 1
fi

if [ ! -x "$BACKEND_BIN" ]; then
    echo "Backend binary not found at $BACKEND_BIN"
    echo "Build first with: make LLVM_CONFIG=<llvm-config-version>"
    exit 1
fi

# Make sure the machine can link 32-bit code before running everything else.
cat > "$TMP_DIR/test32.c" <<'EOF'
int main(void) { return 0; }
EOF
clang -m32 "$TMP_DIR/test32.c" -o "$TMP_DIR/test32"

overall=0
pass_count=0
fail_count=0

run_case() {
    local base="$1"
    local src="$TEST_DIR/$base.c"
    local ll="$TMP_DIR/$base.ll"
    local s="$TMP_DIR/$base.s"
    local expected_bin="$TMP_DIR/$base.expected"
    local actual_bin="$TMP_DIR/$base.out"
    local expected
    local actual
    local input=""

    echo "== $base =="

    clang -S -emit-llvm "$src" -o "$ll"
    "$BACKEND_BIN" "$ll" "$s"

    clang "$RUNTIME_MAIN" "$src" -o "$expected_bin"
    clang -m32 "$RUNTIME_MAIN" "$s" -o "$actual_bin"

    case "$base" in
        sum_n)
            input=$'1\n2\n3\n4\n'
            ;;
        max_n)
            input=$'1\n9\n3\n4\n'
            ;;
    esac

    if [ -n "$input" ]; then
        expected="$(printf "%s" "$input" | "$expected_bin")"
        actual="$(printf "%s" "$input" | "$actual_bin")"
    else
        expected="$("$expected_bin")"
        actual="$("$actual_bin")"
    fi

    if [ "$actual" = "$expected" ]; then
        echo -e "${GREEN}PASS${NC}"
        pass_count=$((pass_count + 1))
    else
        echo -e "${RED}FAIL${NC}"
        echo -e "${YELLOW}Expected:${NC}"
        printf '%s\n' "$expected"
        echo -e "${YELLOW}Got:${NC}"
        printf '%s\n' "$actual"
        overall=1
        fail_count=$((fail_count + 1))
    fi
}

for base in fact fib max_n rem_2 square sum_n; do
    run_case "$base"
done

echo
echo "Summary: $pass_count passed, $fail_count failed"

exit "$overall"
