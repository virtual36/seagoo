SRC_DIR   := src
OBJ_DIR   := obj
LIB_DIR   := lib
DIRS      := $(SRC_DIR) $(OBJ_DIR)

LIB       := libconfig libmagic sqlite3
SRC       := utils.c circular_queue.c config_parser.c dependency_indexing.c main.c lexer.c log.c

DEP       := $(addprefix $(OBJ_DIR)/,$(SRC:.c=.d))
OBJ       := $(DEP:.d=.d.o)

CC        := gcc

CPPFLAGS  := -D_FORTIFY_SOURCE=2 -I$(SRC_DIR) -I$(LIB_DIR)

CFLAGS    := -pipe -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code -Wall -Wno-unused-function \
             $(shell pkg-config --cflags $(LIB)) \
			 -ggdb

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
	rm $(OBJ_DIR)/* src/lexer.c seagoo
	rmdir $(OBJ_DIR)

.PHONY: all debug release clean run

include_lexer.l: # header

$(SRC_DIR)/lexer.c: $(SRC_DIR)/include_lexer.l
	$(LEX) -o $@ $<

$(OBJ_DIR)/%.d.o: $(SRC_DIR)/%.c $(OBJ_DIR)/%.d
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MF $@ -MT $(addsuffix .o,$@) $<

$(DIRS):
	mkdir -p $@
