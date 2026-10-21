BISON ?= bison
FLEX ?= flex
CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra

GENERATED := src/parser.tab.c src/parser.tab.h src/lex.yy.c
SOURCES := src/parser.tab.c src/lex.yy.c src/ast.c src/semantic.c src/catalog.c src/support.c src/json_output.c src/main.c
TARGET := bin/sqlc

all: $(TARGET)

src/parser.tab.c src/parser.tab.h: src/parser.y src/sqlmini.h src/driver.h
	$(BISON) -d -o src/parser.tab.c src/parser.y

src/lex.yy.c: src/lexer.l src/parser.tab.h src/driver.h
	$(FLEX) -o src/lex.yy.c src/lexer.l

$(TARGET): $(SOURCES) src/sqlmini.h src/driver.h src/json_output.h src/parser.tab.h
	mkdir -p bin
	$(CC) $(CFLAGS) -Isrc $(SOURCES) -o $(TARGET)

clean:
	rm -rf bin
	rm -f $(GENERATED)

test: $(TARGET)
	python ../tests/run_tests.py --suite yacc

.PHONY: all clean test
