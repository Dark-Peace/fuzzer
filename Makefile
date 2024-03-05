CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC_DIR = src
BUILD_DIR = .

# List all source files
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Derive object files from source files
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# The final executable
TARGET = fuzzer

# Rule to compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files and generate executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean rule
clean:
	$(RM) $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
