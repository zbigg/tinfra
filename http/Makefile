
DEP_CXXFLAGS=$(shell pkg-config tinfra-test tinfra-regexp --cflags)
DEP_LDLIBS$(shell pkg-config tinfra-test tinfra-regexp --cflags)

CXXFLAGS=-O0 -g -Wall -I.. $(DEP_CXXFLAGS)
LDLIBS=-g $(DEP_LDLIBS)

CC=g++

PROGS=https test_http

all: $(PROGS)

OBJECTS=http.o https.o ../tinfra/connection_handler.o ../tinfra/aio.o ../tinfra/aio_net.o ../tinfra/buffered_aio_adapter.o

https: $(OBJECTS)

test_http: http.o test_http_parser.o ../test_main.o
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(OBJECTS): ../tinfra/*.h http.h

clean:
	rm -rf *.o $(PROGS)
