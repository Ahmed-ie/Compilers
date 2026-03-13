CXX ?= g++
LLVM_CONFIG ?= $(shell if [ -x /opt/homebrew/opt/llvm/bin/llvm-config ]; then echo /opt/homebrew/opt/llvm/bin/llvm-config; \
	elif command -v llvm-config >/dev/null 2>&1; then echo llvm-config; \
	elif command -v llvm-config-18 >/dev/null 2>&1; then echo llvm-config-18; \
	elif command -v llvm-config-17 >/dev/null 2>&1; then echo llvm-config-17; \
	else echo llvm-config; fi)
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra $(shell $(LLVM_CONFIG) --cxxflags)
LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags)
LDLIBS := $(shell $(LLVM_CONFIG) --libs core irreader)

TARGET := optimizer
SRC := main.cpp

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

test: $(TARGET)
	chmod a+x testing.sh
	./testing.sh

clean:
	rm -f $(TARGET)
