CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC_DIR = source
BUILD_DIR = .

# List of source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# List of object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Main target
fuzzer: $(OBJ_FILES)
    $(CC) $(CFLAGS) $^ -o $@

# Rule to compile individual source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
    $(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
    rm -f $(OBJ_FILES) fuzzer
