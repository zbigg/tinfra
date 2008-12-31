CXXFLAGS=-O0 -g -Wall -I.. -DBUILD_UNITTEST
LDLIBS=-g -L$(HOME)/lib -ltinfra -lpthread -lpcre
CC=g++

importer: importer.o tinfra/csv.o

tinfra/csv.o: tinfra/csv.h
