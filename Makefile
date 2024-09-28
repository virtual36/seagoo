CXX       := gcc
CXXFLAGS  := -Wall -g $(INC)

VPATH     := $(OBJECT_PATH) $(SOURCE_PATH)

OBJ_PATH  := obj
SRC_PATH  := src

INC       := -Isrc -Isrc/lib
SRC       := circular_queue.c config_parser.c dependency_indexing.c main.c
OBJ       := $(patsubst %,$(OBJ_PATH)/%,$(SRC:.c=.o))

LDFLAGS   := -L/usr/lib -lconfig

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OBJ_PATH) seagoo

seagoo: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(sort $+) $(LDFLAGS)

$(OBJ_PATH):
	mkdir -p $@

+.NOTPARALLEL: all