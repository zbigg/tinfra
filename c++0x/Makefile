CXXFLAGS=-std=c++0x -O0 -g $(shell pkg-config tinfra-xml --cflags)
LDLIBS=$(shell pkg-config tinfra-xml --libs)
CC=g++


rpc: rpc.o

ws: ws.o

test-rpc: rpc
	./rpc


