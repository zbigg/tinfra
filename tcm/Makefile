CC=g++
LDLIBS=$(shell pkg-config --libs tinfra tinfra-regexp tinfra-test) -g
CXXFLAGS=-g -O0 -Wall -std=c++0x
CPPFLAGS=$(shell pkg-config --cflags tinfra tinfra-regexp tinfra-test) -g
CFLAGS=-fefwe

all: calc tcm_parse lr_parser_test

#programs
tcm_parse: tcm_parse.o lexer.o
calc: calc.o lexer.o
lr_parser_test: lr_parser_test.o lr_parser_detail.o lr_parser.o

#some other rules
tcm_parse.cpp: lexer.h parser.h
lexer.cpp: lexer.h
calc.cpp: lexer.h parser.h
lr_parser_detail.cpp: lr_parser_detail.h lr_parser.h
lr_parser_test.cpp: lr_parser_detail.h lr_parser.h
lr_parser.cpp: lr_parser.h lr_parser_detail.h

clean:
	rm -rf calc tcm_parse lr_parser_test *.o

