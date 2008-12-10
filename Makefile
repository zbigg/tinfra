TINFRA_SRC=/home/zbigg/projects/tinfra/trunk

CXXFLAGS=-O0 -g -Wall -Werror -pedantic -I$(TINFRA_SRC)
LDLIBS=-L$(HOME)/lib -g -ltinfra -lpthread -lpcre
CC=g++

LANG=C
export LANG

http_server: http_server.o lazy_protocol.o

lazy_protocol.o: interruptible.h lazy_protocol.h

posix_signals: posix_signals.o


clean:
	rm -rf *.o ftt