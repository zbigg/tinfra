CXXFLAGS=-O0 -g -Wall -I.. -DBUILD_UNITTEST
LDLIBS=-g -ltinfra -lpthread -lpcre -lunittest++
CC=g++

importer: importer.o tinfra/csv.o

tinfra/csv.o: tinfra/csv.h
