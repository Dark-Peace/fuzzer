all: fuzzer

fuzzer: source/test.c source/help.c source/utils.h
	gcc source/test.c -o fuzzer
