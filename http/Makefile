#CPPFLAGS=
#CXXFLAGS=-O2 -pg -Wall
CXXFLAGS=-O0 -g -Wall -I..
#CFLAGS=-O3 -g
#LDLIBS=-g -pg -L$(HOME)/lib -ltinfra
LDLIBS=-g -L$(HOME)/lib -ltinfra -lpthread -lpcre
CC=g++

PROGS=https

all: $(PROGS)

https: reactor.o protocols.o aio.o

clean:
	rm -rf *.o $(PROGS)
