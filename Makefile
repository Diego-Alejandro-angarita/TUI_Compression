CC       := gcc
CFLAGS   := -Wall -Wextra -Wpedantic -std=c11 -Iinclude
LDLIBS   ?=
DEPS_LDLIBS := -lncurses -lzstd

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/zstd_tui
TEST_COMPRESSION := $(BUILD_DIR)/test_compression
TEST_FILE_IO     := $(BUILD_DIR)/test_file_io
TEST_STATS       := $(BUILD_DIR)/test_stats
TEST_TARGETS     := $(TEST_COMPRESSION) $(TEST_FILE_IO) $(TEST_STATS)

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

# Tests no-TUI: compresion, file I/O y estadisticas.
$(TEST_COMPRESSION): tests/test_compression.c src/compression.c src/stats.c | $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) \
	    tests/test_compression.c \
	    src/compression.c \
	    src/stats.c \
	    -lzstd -o $@

$(TEST_FILE_IO): tests/test_file_io.c src/file_io.c src/stats.c | $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) \
	    tests/test_file_io.c \
	    src/file_io.c \
	    src/stats.c \
	    -o $@

$(TEST_STATS): tests/test_stats.c src/stats.c | $(BUILD_DIR)/.dir
	$(CC) $(CFLAGS) \
	    tests/test_stats.c \
	    src/stats.c \
	    -o $@

test: $(TEST_TARGETS)
	./$(TEST_COMPRESSION)
	./$(TEST_FILE_IO)
	./$(TEST_STATS)

valgrind: $(TEST_TARGETS)
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 \
	         ./$(TEST_COMPRESSION)
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 \
	         ./$(TEST_FILE_IO)
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 \
	         ./$(TEST_STATS)

clean:
	rm -rf $(BUILD_DIR)
