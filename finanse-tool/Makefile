CXXFLAGS=-O0 -g -Wall -I.. -DBUILD_UNITTEST
LDLIBS=-g -ltinfra -lpthread -lpcre -lunittest++
CC=g++

importer: importer.o tinfra/csv.o ../tinfra/lazy_byte_consumer.o

tinfra/csv.o: tinfra/csv.h tinfra/parser.h

../tinfra/lazy_byte_consumer.o: ../tinfra/lazy_byte_consumer.h
