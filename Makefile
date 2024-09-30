OBJ_PATH  := obj
SRC_PATH  := src

INC       := -Isrc -Isrc/lib
HDR       := include_lexer.l circular_queue.h seagoo.h
SRC       := utils.c circular_queue.c config_parser.c dependency_indexing.c main.c lexer.c
OBJ       := $(patsubst %,$(OBJ_PATH)/%,$(SRC:.c=.o))

CC        := gcc
CCFLAGS   := -Wall -g $(INC) -Wno-unused-function
LDFLAGS   := -L/usr/lib -lconfig -lfl -lsqlite3

LEX       := flex
LEXFLAGS  := -o $(SRC_PATH)/lexer.c $(SRC_PATH)/include_lexer.l

YACC      := bison
YACCFLAGS := -d

all: $(OBJ_PATH) seagoo
.NOTPARALLEL: all

seagoo: $(OBJ) $(OBJ_PATH)/lexer.o
	$(CC) $(CCFLAGS) -o $@ $(sort $^) $(LDFLAGS)

run:
	seagoo

clean:
	rm $(OBJ_PATH)/* src/lexer.c seagoo
	rmdir $(OBJ_PATH)

.PHONY: all clean run

# we could automatically generate this dependency information with gcc
$(HDR): # header

$(SRC_PATH)/lexer.c: include_lexer.l
	$(LEX) $(LEXFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c | $(OBJ_PATH)
	$(CC) $(CCFLAGS) -c -o $@ $<

$(OBJ_PATH):
	mkdir -p $@
