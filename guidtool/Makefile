
DEP_CXXFLAGS=$(shell pkg-config tinfra-test tinfra-regexp --cflags)
DEP_LDLIBS=$(shell pkg-config tinfra-test tinfra-regexp --libs)

CXXFLAGS=-O0 -g -Wall $(DEP_CXXFLAGS)
LDLIBS=-g $(DEP_LDLIBS)

CC=g++

PROGS=guidtool

all: $(PROGS)

guidtool: guidtool.o

clean:
	rm -rf *.o $(PROGS)
