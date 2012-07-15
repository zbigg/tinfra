LDLIBS=$(shell pkg-config tinfra tinfra-test --libs)
#LDFLAGS=$(shell pkg-config tinfra --libs)
CXXFLAGS=-O0 -g -Wall -Wextra -Wno-unused
CC=g++

all: mo_full reflect_test proxy_test

mo_full: mo_full.o

reflect_test: reflect_test.o reflect.o

proxy_test: proxy_test.o

clean: 
	rm -rf *.o reflect_test mo_full proxy_test

