CXXFLAGS=-O2 -g -Wall -I.. -DBUILD_UNITTEST
LDLIBS=-g -ltinfra-test -lunittest++ -ltinfra -lpthread -lpcre 
CC=g++

OBJECTS= importer.o tinfra/csv.o
importer: $(OBJECTS)

tinfra/csv.o: tinfra/csv.h ../tinfra/sequence_parser.h ../tinfra/istream_sequence_parser.h


clean:
	rm -rf $(OBJECTS) importer

	
