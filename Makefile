CXXFLAGS=-O0 -g -Wall 
LDLIBS=-g -ltinfra -lpthread
CC=g++

posix_signals: posix_signals.o

#tstring.cpp: tstring.h

clean:
	rm -rf *.o ftt