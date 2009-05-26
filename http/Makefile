#CPPFLAGS=
#CXXFLAGS=-O2 -pg -Wall
CXXFLAGS=-O0 -g -Wall -I..
#CFLAGS=-O3 -g
#LDLIBS=-g -pg -L$(HOME)/lib -ltinfra
LDLIBS=-g -ltinfra -ltinfra-regexp -lpthread -lpcre
CC=g++

PROGS=https

all: $(PROGS)

OBJECTS=http.o https.o ../tinfra/connection_handler.o ../tinfra/aio.o ../tinfra/aio_net.o ../tinfra/buffered_aio_adapter.o

https: $(OBJECTS)

$(OBJECTS): ../tinfra/*.h http.h

clean:
	rm -rf *.o $(PROGS)
