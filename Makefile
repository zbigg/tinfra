TINFRA_SRC=/home/zbigg/projects/tinfra/trunk

CXXFLAGS=-O0 -g -Wall -Werror -pedantic -I$(TINFRA_SRC) -I.
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

colorizer: colorizer.o tinfra/aio.o http/protocol_aio_adapter.o

call_remote: call_remote.o callfwd.o

call_remote.o callfwd.o: callfwd.h

clean:
	rm -rf *.o ftt http/*.o tinfra/*.o

