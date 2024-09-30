OBJ_PATH  := obj
SRC_PATH  := src

INC       := -Isrc -Isrc/lib
SRC       := utils.c circular_queue.c config_parser.c dependency_indexing.c main.c lexer.c
DEP       := $(addprefix $(OBJ_PATH)/,$(SRC:.c=.d))
OBJ       := $(DEP:.d=.d.o)

CC        := gcc
CFLAGS    := -Wall -g $(INC) -Wno-unused-function
LDFLAGS   := -L/usr/lib -lconfig -lfl -lsqlite3

LEX       := flex

YACC      := bison
YACFLAGS := -d

all: $(OBJ_PATH) seagoo

-include $(DEP)

.NOTPARALLEL: all

seagoo: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(sort $^) $(LDFLAGS)

run:
	seagoo

clean:
	rm $(OBJ_PATH)/* src/lexer.c seagoo
	rmdir $(OBJ_PATH)

.PHONY: all clean run

$(SRC_PATH)/%.c:

include_lexer.l: # header

$(SRC_PATH)/lexer.c: $(SRC_PATH)/include_lexer.l
	$(LEX) -o $@ $<

$(OBJ_PATH)/%.d.o: $(SRC_PATH)/%.c $(OBJ_PATH)/%.d
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PATH)/%.d: $(SRC_PATH)/%.c | $(OBJ_PATH)
	$(CC) $(CFLAGS) -MM -MF $@ -MT $(addsuffix .o,$@) $<

$(OBJ_PATH):
	mkdir -p $@
