#CPPFLAGS=
#CXXFLAGS=-O2 -pg -Wall
CXXFLAGS=-O0 -g -Wall -I..
#CFLAGS=-O3 -g
#LDLIBS=-g -pg -L$(HOME)/lib -ltinfra
LDLIBS=-g -ltinfra-test -lunittest++ -ltinfra -lpthread
CC=g++

PROGS=test_aio

all: $(PROGS)

OBJECTS=test_aio.o ../test_main.cpp ../tinfra/connection_handler.o ../tinfra/aio.o ../tinfra/aio_net.o ../http/protocol_aio_adapter.o

test_aio: $(OBJECTS)

$(OBJECTS): ../tinfra/connection_handler.h ../tinfra/generic_factory.h


clean:
	rm -rf *.o $(PROGS)
