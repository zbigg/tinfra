
invoke_bridge_DIR=invoke_bridge
invoke_bridge_SOURCES = invoke_bridge.h

COMPONENTS += invoke_bridge

invoke_bridge_test_DIR=invoke_bridge
invoke_bridge_test_CXXFLAGS = -I$(invoke_bridge_srcdir)
invoke_bridge_test_SOURCES = invoke_bridge_test.cpp
invoke_bridge_test_DEP_LIBS = tinfra-test tinfra
invoke_bridge_test_TESTED_COMPONENT = invoke_bridge

TEST_PROGRAMS += invoke_bridge_test

pkg-config_EXT_LIBS = tinfra tinfra-test

# makefoo

#MAKEFOO_dir=$(shell pkg-config pkg-config --variable=MAKEFOO_dir makefoo)
MAKEFOO_dir=../makefoo
MAKEFOO=$(MAKEFOO_dir)/main.mk

MAKEFOO_USE += native
MAKEFOO_USE += autolib
MAKEFOO_USE += test-program
MAKEFOO_USE += cppcheck

include $(MAKEFOO)


# #CXXFLAGS=-O2 -g -Wall -Werror -I. $(shell pkg-config --cflags tinfra tinfra-test)
# #LDLIBS=$(shell pkg-config --libs tinfra tinfra-test)
# CXXFLAGS=-O2 -g -Wall -Werror -I. 
# LDLIBS=-ltinfra-test -ltinfra
# #LDLIBS=-ltinfra-test -ltinfra -lwsock32 -Wl,--enable-auto-import
# 
# CC=g++
# 
# all: unittests
# 
# LANG=C
# export LANG
# 
# TINFRA_SANDBOX_OBJECTS=\
	# tinfra/aio.o \
	# tinfra/aio_net.o \
	# tinfra/buffered_aio_adapter.o \
# 
# COMMON_OBJECTS=\
	# tinfra/callfwd.o
# 
# tzfile_read: tzfile_read.o
# 
# editor_server: editor_server.o
# 
# http_server: http_server.o 
# 
# tinfra/aio.o: tinfra/aio.h
# tinfra/aio_net.o: tinfra/aio.h tinfra/aio_net.h
# 
# posix_signals: posix_signals.o
# 
# list_files_generator: list_files_generator.o
# 
# colorizer: colorizer.o tinfra/aio.o tinfra/buffered_aio_adapter.o
# 
# callfwd_test: callfwd_test.o callfwd.o
# 
# callfwd_test.o callfwd.o: callfwd.h callfwd_detail.h
# 
# test_gui: test_gui.o tinfra/gui/context.o
# 
# UNITESTS_OBJECTS=\
	# tests/callfwd_test.o \
	# tests/callback_test.o
		# 
# unittests: unittests.o $(UNITESTS_OBJECTS) $(COMMON_OBJECTS)
# 
# async_fs_poc: async_fs_poc.o $(TINFRA_SANDBOX_OBJECTS)
# 
# clean:
	# rm -rf *.o ftt http/*.o tinfra/*.o tests/*.o
