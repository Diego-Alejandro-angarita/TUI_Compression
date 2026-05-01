CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Iinclude
LDLIBS ?=
DEPS_LDLIBS := -lncurses -lzstd

BUILD_DIR := build
TARGET := $(BUILD_DIR)/zstd_tui
SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=$(BUILD_DIR)/%.o)

.PHONY: build build-with-deps run test valgrind clean

build: $(OBJ)

$(TARGET): $(OBJ) | $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/.dir:
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_DIR)/.dir

build-with-deps: LDLIBS = $(DEPS_LDLIBS)
build-with-deps: clean $(TARGET)

run: build-with-deps
	./$(TARGET)

test:
	@echo "No hay pruebas implementadas todavia."

valgrind: build-with-deps
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
