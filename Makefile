all: fuzzer

fuzzer: source/test.c
  gcc source/test.c -o fuzzer
