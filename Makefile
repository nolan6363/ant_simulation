CC = gcc
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

CFLAGS = -Werror -g -fno-omit-frame-pointer -O2 -pthread -MMD -MP $(shell pkg-config --cflags sdl3 sdl3-ttf)
LDLIBS = $(shell pkg-config --libs sdl3 sdl3-ttf) -lm

SRCS = $(wildcard $(SRC_DIR)/*.c)
PAR_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/par/%.o, $(SRCS))
SEQ_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/seq/%.o, $(SRCS))

all: $(BIN_DIR)/main $(BIN_DIR)/main_seq

$(OBJ_DIR)/par/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -fopenmp -c $< -o $@

$(OBJ_DIR)/seq/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/main: $(PAR_OBJS)
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) -fopenmp $^ -o $@ $(LDLIBS)

$(BIN_DIR)/main_seq: $(SEQ_OBJS)
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

run: $(BIN_DIR)/main
	@./$(BIN_DIR)/main

test: all
	@./test.sh

clean:
	@rm -rf "$(BIN_DIR)" "$(OBJ_DIR)"

-include $(PAR_OBJS:.o=.d) $(SEQ_OBJS:.o=.d)

.PHONY: all run test clean
