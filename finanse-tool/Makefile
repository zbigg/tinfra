CXXFLAGS=-O2 -g -Wall -I.. -I. -DBUILD_UNITTEST
LDLIBS=-g $(shell pkg-config --libs tinfra tinfra-test)
CC=g++

COMMON_OBJECTS=  tinfra/csv.o

all: importer csv_decompose
importer: importer.o $(COMMON_OBJECTS)
csv_decompose: csv_decompose.o $(COMMON_OBJECTS)

tinfra/csv.o: tinfra/csv.h ../tinfra/sequence_parser.h ../tinfra/istream_sequence_parser.h


clean:
	rm -rf *.o tinfra/*.o importer

	
