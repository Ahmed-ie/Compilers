CXX = g++
LLVM_CONFIG ?= $(if $(wildcard /opt/homebrew/opt/llvm/bin/llvm-config),/opt/homebrew/opt/llvm/bin/llvm-config,llvm-config)

TARGET = backend

CXXFLAGS += -std=c++17 -O2 -Wall -Wextra $(shell $(LLVM_CONFIG) --cxxflags)
LDFLAGS += $(shell $(LLVM_CONFIG) --ldflags)
LDLIBS += $(shell $(LLVM_CONFIG) --system-libs --libs core irreader support)

OBJS = main.o

.PHONY: all clean smoke-test

all: $(TARGET)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o $@

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

smoke-test: $(TARGET)
	./$(TARGET) /tmp/minic_backend_test.ll /tmp/minic_backend_test.s
	@echo "Wrote /tmp/minic_backend_test.s"

clean:
	rm -f $(TARGET) $(OBJS) /tmp/minic_backend_test.s
