SRC_PATH  := src
OBJ_PATH  := obj
LIB_PATH  := lib
DIRS      := $(SRC_PATH) $(OBJ_PATH)

LIB       := libconfig libmagic libfl sqlite3
SRC       := utils.c circular_queue.c config_parser.c dependency_indexing.c main.c lexer.c log.c

DEP       := $(addprefix $(OBJ_PATH)/,$(SRC:.c=.d))
OBJ       := $(DEP:.d=.d.o)

CC        := gcc

CPPFLAGS  := -D_FORTIFY_SOURCE=2 -I$(SRC_PATH) -I$(LIB_PATH)

CFLAGS    := -pipe -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code -Wall -Wno-unused-function \
             $(shell pkg-config --cflags $(LIB))

LDFLAGS   := -L/usr/lib \
             $(shell pkg-config --libs $(LIB))

LEX       := flex

YACC      := bison
YACFLAGS  := -d

all: release

debug: CFLAGS += -ggdb -O0
release: CFLAGS += -O3

debug release: $(DIRS) seagoo

.NOTPARALLEL: debug release

-include $(DEP)

seagoo: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(sort $^) $(LDFLAGS)

run:
	seagoo

clean:
	rm $(OBJ_PATH)/* src/lexer.c seagoo
	rmdir $(OBJ_PATH)

.PHONY: all debug release clean run

include_lexer.l: # header

$(SRC_PATH)/lexer.c: $(SRC_PATH)/include_lexer.l
	$(LEX) -o $@ $<

$(OBJ_PATH)/%.d.o: $(SRC_PATH)/%.c $(OBJ_PATH)/%.d
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OBJ_PATH)/%.d: $(SRC_PATH)/%.c | $(OBJ_PATH)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MF $@ -MT $(addsuffix .o,$@) $<

$(DIRS):
	mkdir -p $@
