CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -Iinclude $(CPPFLAGS)
LDFLAGS ?=
LDLIBS ?=

BUILD_DIR := build
BIN_DIR := bin
TARGET := $(BIN_DIR)/editor_secure_stub
BUILD_STAMP := $(BUILD_DIR)/.dir
BIN_STAMP := $(BIN_DIR)/.dir

SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=$(BUILD_DIR)/%.o)

TEST_SRC := $(wildcard tests/test_*.c)
TEST_BIN := $(TEST_SRC:tests/%.c=$(BUILD_DIR)/%)
TEST_SUPPORT_SRC := $(filter-out src/main.c,$(SRC))
TEST_SUPPORT_OBJ := $(TEST_SUPPORT_SRC:src/%.c=$(BUILD_DIR)/%.o)

.PHONY: all build build-with-deps run test valgrind clean

all: $(TARGET)

build: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_STAMP) $(BUILD_STAMP)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_%: tests/test_%.c $(TEST_SUPPORT_OBJ) | $(BUILD_STAMP)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_STAMP):
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_STAMP)

$(BIN_STAMP):
	mkdir -p $(BIN_DIR)
	touch $(BIN_STAMP)

build-with-deps: build

run: build
	./$(TARGET)

test: $(TEST_BIN)
	@for test_bin in $(TEST_BIN); do ./$$test_bin || exit 1; done

valgrind: test
	@for test_bin in $(TEST_BIN); do valgrind --leak-check=full --show-leak-kinds=all ./$$test_bin || exit 1; done

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
