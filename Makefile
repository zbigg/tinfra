TINFRA_SRC=/home/zbigg/projects/tinfra/trunk

CXXFLAGS=-O2 -g -Wall -Werror -I. $(shell pkg-config --cflags tinfra tinfra-test)
LDLIBS=$(shell pkg-config --libs tinfra tinfra-test)
CC=g++

LANG=C
export LANG

editor_server: editor_server.o

http_server: http_server.o tinfra/lazy_protocol.o tinfra/aio.o tinfra/aio_net.o

tinfra/lazy_protocol.o: tinfra/interruptible.h tinfra/lazy_protocol.h
tinfra/aio.o: tinfra/aio.h
tinfra/aio_net.o: tinfra/aio.h tinfra/aio_net.h

posix_signals: posix_signals.o

list_files_generator: list_files_generator.o

colorizer: colorizer.o tinfra/aio.o tinfra/buffered_aio_adapter.o

callfwd_test: callfwd_test.o callfwd.o

callfwd_test.o callfwd.o: callfwd.h callfwd_detail.h

test_gui: test_gui.o tinfra/gui/context.o

unittests: unittests.o tests/shared_ptr_test.o tests/runner_test.o tests/callfwd_test.o tinfra/callfwd.o

tests/shared_ptr_test.o: tinfra/shared_ptr.h
tests/runner_test.o: tinfra/thread_pool.h

async_fs_poc: async_fs_poc.o callfwd.o

clean:
	rm -rf *.o ftt http/*.o tinfra/*.o tests/*.o

