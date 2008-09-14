#CPPFLAGS=
#CXXFLAGS=-O2 -pg -Wall
CXXFLAGS=-O0 -g -Wall
#CFLAGS=-O3 -g
#LDLIBS=-g -pg -L$(HOME)/lib -ltinfra
LDLIBS=-g -L$(HOME)/lib -ltinfra -lpthread
CC=g++

PROGS=reactor autosh

all: $(PROGS)

reactor: reactor.o protocols.o aio.o

autosh: autosh.o protocols.o aio.o

clean:
	rm -rf *.o $(PROGS)
