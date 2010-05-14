CC=g++
LDLIBS=$(shell pkg-config --libs tinfra tinfra-regexp)

tcm_parse: tcm_parse.o lexer.o

tcm_parse.cpp: lexer.h parser.h
lexer.cpp: lexer.h
