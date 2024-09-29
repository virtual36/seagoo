CXX       := gcc
CXXFLAGS  := -Wall -g $(INC)

LEX       := flex
LEXFLAGS  := -o lexer.c include_lexer.l

OBJ_PATH  := obj
SRC_PATH  := src

INC       := -Isrc -Isrc/lib
SRC       := circular_queue.c config_parser.c dependency_indexing.c main.c
OBJ       := $(patsubst %,$(OBJ_PATH)/%,$(SRC:.c=.o))

LDFLAGS   := -L/usr/lib -lconfig -lfl

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c | $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

lexer.c: include_lexer.l
	$(LEX) $(LEXFLAGS)

all: $(OBJ_PATH) seagoo

seagoo: $(OBJ) lexer.o
	$(CXX) $(CXXFLAGS) -o $@ $(sort $^) $(LDFLAGS)

$(OBJ_PATH):
	mkdir -p $@

.PHONY: all clean

clean:
	rm -rf $(OBJ_PATH)/* lexer.c seagoo