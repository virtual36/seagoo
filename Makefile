INC       := -Isrc -Isrc/lib

CC        := gcc
CCFLAGS   := -Wall -g $(INC) -Wno-unused-function

LEX       := flex
LEXFLAGS  := -o src/lexer.c src/include_lexer.l

YACC      := bison
YACCFLAGS := -d

OBJ_PATH  := obj
SRC_PATH  := src

SRC       := utils.c circular_queue.c config_parser.c dependency_indexing.c main.c
OBJ       := $(patsubst %,$(OBJ_PATH)/%,$(SRC:.c=.o))

LDFLAGS   := -L/usr/lib -lconfig -lfl -lsqlite3

seagoo: $(OBJ) $(OBJ_PATH)/lexer.o
	$(CC) $(CCFLAGS) -o $@ $(sort $^) $(LDFLAGS)

src/lexer.c: src/include_lexer.l
	$(LEX) $(LEXFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c | $(OBJ_PATH)
	$(CC) $(CCFLAGS) -c -o $@ $<

$(OBJ_PATH)/lexer.o: src/lexer.c
	$(CC) $(CCFLAGS) -c -o $@ $<

all: $(OBJ_PATH) seagoo

$(OBJ_PATH):
	mkdir -p $@

.PHONY: all clean run

clean:
	rm -rf $(OBJ_PATH)/* src/lexer.c seagoo

run:
	seagoo
