
PKG_CONFIG_PACKAGES=tinfra tinfra-test

CXXFLAGS = -g -O0 $(shell pkg-config $(PKG_CONFIG_PACKAGES) --cflags)
LDLIBS = -g $(shell pkg-config $(PKG_CONFIG_PACKAGES) --libs)
CC=g++

invoke_bridge_test: invoke_bridge_test.o # invoke_bridge.o

clean:
	rm -rf invoke_bridge *.o

