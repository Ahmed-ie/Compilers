CXX = g++
LEX = flex
YACC = bison
LLVM_CONFIG ?= $(shell if [ -x /opt/homebrew/opt/llvm/bin/llvm-config ]; then echo /opt/homebrew/opt/llvm/bin/llvm-config; \
	elif command -v llvm-config >/dev/null 2>&1; then echo llvm-config; \
	elif command -v llvm-config-18 >/dev/null 2>&1; then echo llvm-config-18; \
	elif command -v llvm-config-17 >/dev/null 2>&1; then echo llvm-config-17; \
	else echo llvm-config; fi)

TARGET = ir_builder

YACC_SRC = ../frontend/yac.y
YACC_C = yac.tab.c
YACC_H = yac.tab.h
LEX_SRC = ../frontend/lex.l
LEX_C = lex.yy.c

CXXFLAGS += -std=c++17 -O2 -Wall -Wextra -I../frontend $(shell $(LLVM_CONFIG) --cflags)
LDFLAGS += $(shell $(LLVM_CONFIG) --ldflags)
LDLIBS += $(shell $(LLVM_CONFIG) --libs core)

OBJS = yac.tab.o lex.yy.o ast.o semantic_analysis.o ir_builder.o main.o

all: $(TARGET)

$(YACC_C) $(YACC_H): $(YACC_SRC)
	$(YACC) -d -o $(YACC_C) $(YACC_SRC)

$(LEX_C): $(LEX_SRC) $(YACC_H)
	$(LEX) -o $(LEX_C) $(LEX_SRC)

yac.tab.o: $(YACC_C) $(YACC_H)
	$(CXX) $(CXXFLAGS) -c $(YACC_C)

lex.yy.o: $(LEX_C) $(YACC_H)
	$(CXX) $(CXXFLAGS) -c $(LEX_C)

ast.o: ../frontend/ast.c ../frontend/ast.h
	$(CXX) $(CXXFLAGS) -c ../frontend/ast.c -o $@

semantic_analysis.o: ../frontend/semantic_analysis.c ../frontend/semantic_analysis.h ../frontend/ast.h
	$(CXX) $(CXXFLAGS) -c ../frontend/semantic_analysis.c -o $@

ir_builder.o: ir_builder.cpp ir_builder.h ../frontend/ast.h
	$(CXX) $(CXXFLAGS) -c ir_builder.cpp

main.o: main.cpp ir_builder.h ../frontend/ast.h ../frontend/semantic_analysis.h $(YACC_H)
	$(CXX) $(CXXFLAGS) -c main.cpp

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) ../tests/builder_tests/p1.c /tmp/p1.ll
	@echo "Wrote /tmp/p1.ll"

test-builder: $(TARGET)
	@for f in ../tests/builder_tests/p*.c; do \
		out=/tmp/$$(basename $$f .c).ll; \
		echo "== $$f -> $$out =="; \
		./$(TARGET) "$$f" "$$out" || exit $$?; \
	done

clean:
	rm -f $(TARGET) *.o $(YACC_C) $(YACC_H) $(LEX_C) yac.output /tmp/p1.ll /tmp/p2.ll /tmp/p3.ll /tmp/p4.ll /tmp/p5.ll /tmp/p6.ll
