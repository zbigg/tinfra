#CPPFLAGS=
#CXXFLAGS=-O2 -pg -Wall
CXXFLAGS=-O0 -g -Wall
#CFLAGS=-O3 -g
#LDLIBS=-g -pg -L$(HOME)/lib -ltinfra
LDLIBS=-g -L$(HOME)/lib -ltinfra -lpthread -lpcre
CC=g++

PROGS=scope

all: $(PROGS)

scope: scope.o

clean:
	rm -rf *.o $(PROGS)
