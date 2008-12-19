TINFRA_SRC=/home/zbigg/projects/tinfra/trunk

CXXFLAGS=-O4 -Wall -Werror -pedantic -I$(TINFRA_SRC) -I.
LDLIBS=-L$(HOME)/lib -g -ltinfra -lpthread -lpcre
CC=g++

LANG=C
export LANG

http_server: http_server.o tinfra/lazy_protocol.o tinfra/aio.o tinfra/aio_net.o

tinfra/lazy_protocol.o: tinfra/interruptible.h tinfra/lazy_protocol.h
tinfra/aio.o: tinfra/aio.h
tinfra/aio_net.o: tinfra/aio.h tinfra/aio_net.h

posix_signals: posix_signals.o

list_files_generator: list_files_generator.o

clean:
	rm -rf *.o ftt
