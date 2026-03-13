#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/optimizations/optimizer"
TEST_DIR="$ROOT/tests/optimizations"

pass_count=0
fail_count=0

run_diff_test() {
    local label="$1"
    local mode="$2"
    local input="$3"
    local expected="$4"
    local out="/tmp/${label}.out.ll"

    if [ "$mode" = "local" ]; then
        "$BIN" --local "$input" > "$out"
    else
        "$BIN" "$input" > "$out"
    fi

    if diff -u -I "ModuleID" -I "PIC Level" "$out" "$expected" >/dev/null; then
        echo "  $label: PASS"
        pass_count=$((pass_count + 1))
    else
        echo "  $label: FAIL"
        fail_count=$((fail_count + 1))
    fi
}

if [ ! -x "$BIN" ]; then
    echo "Optimizer binary not found at $BIN"
    echo "Build first with: make"
    exit 1
fi

echo "Local tests..."
run_diff_test "cfold_add" "local" "$TEST_DIR/cfold_add.ll" "$TEST_DIR/cfold_add_opt.ll"
run_diff_test "cfold_mul" "local" "$TEST_DIR/cfold_mul.ll" "$TEST_DIR/cfold_mul_opt.ll"
run_diff_test "cfold_sub" "local" "$TEST_DIR/cfold_sub.ll" "$TEST_DIR/cfold_sub_opt.ll"
run_diff_test "p2_common_subexpr" "local" "$TEST_DIR/p2_common_subexpr.ll" "$TEST_DIR/p2_common_subexpr_opt.ll"

echo "Global tests..."
run_diff_test "p3_const_prop" "global" "$TEST_DIR/p3_const_prop.ll" "$TEST_DIR/p3_const_prop_opt.ll"
run_diff_test "p4_const_prop" "global" "$TEST_DIR/p4_const_prop.ll" "$TEST_DIR/p4_const_prop_opt.ll"
run_diff_test "p5_const_prop" "global" "$TEST_DIR/p5_const_prop.ll" "$TEST_DIR/p5_const_prop_opt.ll"

echo "Summary: $pass_count passed, $fail_count failed."
[ "$fail_count" -eq 0 ]
