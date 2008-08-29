#CPPFLAGS=
#CXXFLAGS=-O2 -pg -Wall
CXXFLAGS=-O0 -g -Wall
#CFLAGS=-O3 -g
#LDLIBS=-g -pg -L$(HOME)/lib -ltinfra
LDLIBS=-g -L$(HOME)/lib -ltinfra -lpthread
CC=g++

reactor: reactor.o	


reader: reader.o

stackstring: stackstring.o

clean:
	rm -rf *.o reactor