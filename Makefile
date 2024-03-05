# Makefile for building the fuzzer program

# Compiler options
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Source files and output executable
SRC_DIR = source
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(SRC_FILES:.c=.o)
EXECUTABLE = fuzzer

# Default target
all: $(EXECUTABLE)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files into executable
$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Clean target to remove generated files
clean:
	rm -f $(EXECUTABLE) $(OBJ_FILES)
