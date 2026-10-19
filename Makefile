CC = gcc
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

CFLAGS = -Werror $(shell pkg-config --cflags sdl3 sdl3-ttf)
LDLIBS = $(shell pkg-config --libs sdl3 sdl3-ttf) -lm

LIB_SRCS = $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRCS))

all: $(BIN_DIR)/main

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/main: $(SRC_DIR)/main.c $(LIB_OBJS) | $(BIN_DIR) $(OBJ_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

run: $(BIN_DIR)/main
	@./$(BIN_DIR)/main

clean:
	@rm -rf "$(BIN_DIR)/main" "$(OBJ_DIR)"

.PHONY: all run clean distclean
