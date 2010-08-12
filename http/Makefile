
DEP_CXXFLAGS=$(shell pkg-config tinfra-test tinfra-regexp --cflags)
DEP_LDLIBS=$(shell pkg-config tinfra-test tinfra-regexp --libs)

CXXFLAGS=-O0 -g -Wall -I.. $(DEP_CXXFLAGS)
LDLIBS=-g $(DEP_LDLIBS)

CC=g++

SOURCES=http.cpp ../tinfra/connection_handler.cpp

test_http: http.o test_http_parser.o ../unittests.o
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(SOURCES): ../tinfra/*.h http.h

clean:
	rm -rf *.o $(PROGS)
