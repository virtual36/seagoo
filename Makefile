CXX       := gcc
CXXFLAGS  := -Wall -g $(INC) -Wno-unused-function

LEX       := flex
LEXFLAGS  := -o src/lexer.c src/include_lexer.l

YACC      := bison
YACCFLAGS := -d -o src/include_parser.tab.c src/include_parser.y

OBJ_PATH  := obj
SRC_PATH  := src

INC       := -Isrc -Isrc/lib
SRC       := circular_queue.c config_parser.c dependency_indexing.c main.c
OBJ       := $(patsubst %,$(OBJ_PATH)/%,$(SRC:.c=.o))

LDFLAGS   := -L/usr/lib -lconfig -lfl -lsqlite3

src/include_parser.tab.c src/include_parser.tab.h: src/include_parser.y
	$(YACC) $(YACCFLAGS)

src/lexer.c: src/include_lexer.l src/include_parser.tab.h
	$(LEX) $(LEXFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c | $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_PATH)/lexer.o: src/lexer.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_PATH)/parser.o: src/include_parser.tab.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OBJ_PATH) seagoo

seagoo: $(OBJ) $(OBJ_PATH)/lexer.o $(OBJ_PATH)/parser.o
	$(CXX) $(CXXFLAGS) -o $@ $(sort $^) $(LDFLAGS)

$(OBJ_PATH):
	mkdir -p $@

.PHONY: all clean

clean:
	rm -rf $(OBJ_PATH)/* src/lexer.c src/include_parser.tab.c src/include_parser.tab.h seagoo
