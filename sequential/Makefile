CC = gcc
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

CFLAGS = -Werror -g -fno-omit-frame-pointer -O2 -pthread -MMD -MP $(shell pkg-config --cflags sdl3 sdl3-ttf)
LDLIBS = $(shell pkg-config --libs sdl3 sdl3-ttf) -lm

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(BIN_DIR)/main

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/main: $(OBJS)
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

run: $(BIN_DIR)/main
	@./$(BIN_DIR)/main

clean:
	@rm -rf "$(BIN_DIR)" "$(OBJ_DIR)"

-include $(OBJS:.o=.d)

.PHONY: all run clean
