CC=g++
LDLIBS=$(shell pkg-config --libs tinfra tinfra-regexp) -g
CXXFlAGS=-g -O0 -Wall
CFLAGS=-fefwe

tcm_parse: tcm_parse.o lexer.o
calc: calc.o lexer.o

tcm_parse.cpp: lexer.h parser.h
lexer.cpp: lexer.h
calc.cpp: lexer.h parser.h

clean:
	rm -rf calc tcm_parse *.o

