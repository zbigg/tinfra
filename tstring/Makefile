CXXFLAGS=-O0 -g -Wall -I$(HOME)/include `pcre-config --cflags`
LDLIBS=-g -L$(HOME)/lib -ltinfra -lpthread -lpcre
CC=g++

tstring: tstring.o

#tstring.cpp: tstring.h

regex_pcre: regex_pcre.o regexp.o

filter: filter.o

clean:
	rm -rf *.o tstring