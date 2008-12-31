#CPPFLAGS=
#CXXFLAGS=-O2 -pg -Wall
CXXFLAGS=-O0 -g -Wall -I..
#CFLAGS=-O3 -g
#LDLIBS=-g -pg -L$(HOME)/lib -ltinfra
LDLIBS=-g -ltinfra -lpthread -lpcre
CC=g++

PROGS=https

all: $(PROGS)

OBJECTS=https.o ../tinfra/connection_handler.o ../tinfra/aio.o ../tinfra/aio_net.o protocol_aio_adapter.o

https: $(OBJECTS)

$(OBJECTS): ../tinfra/connection_handler.h ../tinfra/generic_factory.h

https.o protocol_aio_adapter.o: protocol_aio_adapter.h

clean:
	rm -rf *.o $(PROGS)
