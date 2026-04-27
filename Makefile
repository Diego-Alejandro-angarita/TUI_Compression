CC       := gcc
CFLAGS   := -Wall -Wextra -Wpedantic -std=c11 -Iinclude
LDLIBS   ?=
DEPS_LDLIBS := -lncurses -lzstd

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/zstd_tui

SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=$(BUILD_DIR)/%.o)

.PHONY: build build-with-deps run test valgrind clean

build: $(TARGET)

$(TARGET): $(OBJ) | $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/.dir:
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_DIR)/.dir

build-with-deps: LDLIBS = $(DEPS_LDLIBS)
build-with-deps: clean $(TARGET)

run: build
	./$(TARGET)

# ── Tests de compresión (Persona 2) ────────────────────────────────────────
# No depende de ncurses; solo necesita -lzstd.
test: $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) \
	    tests/test_compression.c \
	    src/compression.c \
	    src/stats.c \
	    -lzstd -o $(BUILD_DIR)/test_compression
	./$(BUILD_DIR)/test_compression

valgrind: $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) \
	    tests/test_compression.c \
	    src/compression.c \
	    src/stats.c \
	    -lzstd -o $(BUILD_DIR)/test_compression
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 \
	         ./$(BUILD_DIR)/test_compression

clean:
	rm -rf $(BUILD_DIR)