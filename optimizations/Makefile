CXX ?= g++
LLVM_CONFIG ?= $(if $(wildcard /opt/homebrew/opt/llvm/bin/llvm-config),/opt/homebrew/opt/llvm/bin/llvm-config,llvm-config)
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra $(shell $(LLVM_CONFIG) --cxxflags)
LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags)
LDLIBS := $(shell $(LLVM_CONFIG) --libs core irreader)

TARGET := optimizer
SRC := main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

test: $(TARGET)
	@set -e; \
	pass=0; fail=0; \
	echo "Local tests..."; \
	./$(TARGET) --local ../tests/optimizations/cfold_add.ll > /tmp/cfold_add.out.ll; \
	if diff -u -I "ModuleID" -I "PIC Level" /tmp/cfold_add.out.ll ../optimizer_test_results/cfold_add_opt.ll >/dev/null; then echo "  cfold_add: PASS"; pass=$$((pass+1)); else echo "  cfold_add: FAIL"; fail=$$((fail+1)); fi; \
	./$(TARGET) --local ../tests/optimizations/cfold_mul.ll > /tmp/cfold_mul.out.ll; \
	if diff -u -I "ModuleID" -I "PIC Level" /tmp/cfold_mul.out.ll ../optimizer_test_results/cfold_mul_opt.ll >/dev/null; then echo "  cfold_mul: PASS"; pass=$$((pass+1)); else echo "  cfold_mul: FAIL"; fail=$$((fail+1)); fi; \
	./$(TARGET) --local ../tests/optimizations/cfold_sub.ll > /tmp/cfold_sub.out.ll; \
	if diff -u -I "ModuleID" -I "PIC Level" /tmp/cfold_sub.out.ll ../optimizer_test_results/cfold_sub_opt.ll >/dev/null; then echo "  cfold_sub: PASS"; pass=$$((pass+1)); else echo "  cfold_sub: FAIL"; fail=$$((fail+1)); fi; \
	./$(TARGET) --local ../tests/optimizations/p2_common_subexpr.ll > /tmp/p2_common_subexpr.out.ll; \
	if diff -u -I "ModuleID" -I "PIC Level" /tmp/p2_common_subexpr.out.ll ../optimizer_test_results/p2_common_subexpr_opt.ll >/dev/null; then echo "  p2_common_subexpr: PASS"; pass=$$((pass+1)); else echo "  p2_common_subexpr: FAIL"; fail=$$((fail+1)); fi; \
	echo "Global tests..."; \
	./$(TARGET) ../tests/optimizations/p3_const_prop.ll > /tmp/p3_const_prop.out.ll; \
	if diff -u -I "ModuleID" -I "PIC Level" /tmp/p3_const_prop.out.ll ../optimizer_test_results/p3_const_prop_opt.ll >/dev/null; then echo "  p3_const_prop: PASS"; pass=$$((pass+1)); else echo "  p3_const_prop: FAIL"; fail=$$((fail+1)); fi; \
	./$(TARGET) ../tests/optimizations/p4_const_prop.ll > /tmp/p4_const_prop.out.ll; \
	if diff -u -I "ModuleID" -I "PIC Level" /tmp/p4_const_prop.out.ll ../optimizer_test_results/p4_const_prop_opt.ll >/dev/null; then echo "  p4_const_prop: PASS"; pass=$$((pass+1)); else echo "  p4_const_prop: FAIL"; fail=$$((fail+1)); fi; \
	./$(TARGET) ../tests/optimizations/p5_const_prop.ll > /tmp/p5_const_prop.out.ll; \
	if diff -u -I "ModuleID" -I "PIC Level" /tmp/p5_const_prop.out.ll ../optimizer_test_results/p5_const_prop_opt.ll >/dev/null; then echo "  p5_const_prop: PASS"; pass=$$((pass+1)); else echo "  p5_const_prop: FAIL"; fail=$$((fail+1)); fi; \
	echo "Summary: $$pass passed, $$fail failed."

clean:
	rm -f $(TARGET)

.PHONY: all clean test
