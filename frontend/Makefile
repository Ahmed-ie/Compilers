CC = g++
LEX = flex
YACC = bison

TARGET = parser

YACC_SRC = yac.y
YACC_C = yac.tab.c
YACC_H = yac.tab.h
LEX_SRC = lex.l
LEX_C = lex.yy.c

OBJS = yac.tab.o lex.yy.o ast.o semantic_analysis.o main.o

all: $(TARGET)

$(YACC_C) $(YACC_H): $(YACC_SRC)
	$(YACC) -d $(YACC_SRC)

$(LEX_C): $(LEX_SRC) $(YACC_H)
	$(LEX) $(LEX_SRC)

yac.tab.o: $(YACC_C) $(YACC_H)
	$(CC) -c $(YACC_C)

lex.yy.o: $(LEX_C) $(YACC_H)
	$(CC) -c $(LEX_C)

ast.o: ast.c ast.h
	$(CC) -c ast.c

semantic_analysis.o: semantic_analysis.c semantic_analysis.h ast.h
	$(CC) -c semantic_analysis.c

main.o: main.c ast.h $(YACC_H)
	$(CC) -c main.c

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

parser-tests: $(TARGET)
	@for f in ../tests/parser/p*.c; do \
		echo "== $$f =="; \
		./$(TARGET) "$$f"; \
		echo; \
	done

semantic-tests: $(TARGET)
	@for f in ../tests/semantic/p*.c; do \
		echo "== $$f =="; \
		./$(TARGET) "$$f"; \
		echo; \
	done
clean:
	rm -f $(TARGET) *.o $(YACC_C) $(YACC_H) $(LEX_C) yac.output
