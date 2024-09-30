OBJ_PATH  := obj
SRC_PATH  := src

LIB       := libconfig libmagic libfl sqlite3
SRC       := utils.c circular_queue.c config_parser.c dependency_indexing.c main.c lexer.c
DEP       := $(addprefix $(OBJ_PATH)/,$(SRC:.c=.d))
OBJ       := $(DEP:.d=.d.o)

CC        := gcc

CPPFLAGS  := -D_FORTIFY_SOURCE=2 -Isrc -Isrc/lib

CFLAGS    := -pipe -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code -Wall -Wno-unused-function \
             $(shell pkg-config --cflags $(LIB))

LDFLAGS   := -L/usr/lib \
             $(shell pkg-config --libs $(LIB))

LEX       := flex

YACC      := bison
YACFLAGS  := -d

debug: CFLAGS += -ggdb -O0
release: CFLAGS += -O3

all: release
debug release: $(OBJ_PATH) seagoo
.NOTPARALLEL: all

-include $(DEP)

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
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OBJ_PATH)/%.d: $(SRC_PATH)/%.c | $(OBJ_PATH)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MF $@ -MT $(addsuffix .o,$@) $<

$(OBJ_PATH):
	mkdir -p $@
