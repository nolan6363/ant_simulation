CC = gcc
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

CFLAGS = -Werror -Wall $(shell pkg-config --cflags sdl3)
LDLIBS = $(shell pkg-config --libs sdl3)

all: $(BIN_DIR)/main

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(BIN_DIR)/main: $(SRC_DIR)/main.c | $(BIN_DIR) $(OBJ_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

run: $(BIN_DIR)/main
	@./$(BIN_DIR)/main

clean:
	@rm -rf "$(BIN_DIR)/main" "$(OBJ_DIR)"

.PHONY: all run clean distclean
