CXXFLAGS=-O9 -g -Wall -I.. -DBUILD_UNITTEST
LDLIBS=-g -ltinfra -lpthread -lpcre -lunittest++
CC=g++

OBJECTS= importer.o tinfra/csv.o
importer: $(OBJECTS)

tinfra/csv.o: tinfra/csv.h ../tinfra/sequence_parser.h ../tinfra/istream_sequence_parser.h


clean:
	rm -rf $(OBJECTS) importer

	
