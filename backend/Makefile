CXX = g++
LLVM_CONFIG ?= $(shell if [ -x /opt/homebrew/opt/llvm/bin/llvm-config ]; then echo /opt/homebrew/opt/llvm/bin/llvm-config; \
	elif command -v llvm-config >/dev/null 2>&1; then echo llvm-config; \
	elif command -v llvm-config-18 >/dev/null 2>&1; then echo llvm-config-18; \
	elif command -v llvm-config-17 >/dev/null 2>&1; then echo llvm-config-17; \
	else echo llvm-config; fi)

TARGET = backend

CXXFLAGS += -std=c++17 -O2 -Wall -Wextra $(shell $(LLVM_CONFIG) --cxxflags)
LDFLAGS += $(shell $(LLVM_CONFIG) --ldflags)
LDLIBS += $(shell $(LLVM_CONFIG) --system-libs --libs core irreader support)

OBJS = main.o

.PHONY: all clean smoke-test test

all: $(TARGET)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o $@

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

smoke-test: $(TARGET)
	./$(TARGET) /tmp/minic_backend_test.ll /tmp/minic_backend_test.s
	@echo "Wrote /tmp/minic_backend_test.s"

test: $(TARGET)
	chmod a+x testing.sh
	./testing.sh

clean:
	rm -f $(TARGET) $(OBJS) /tmp/minic_backend_test.s
