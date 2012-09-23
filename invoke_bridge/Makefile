
PKG_CONFIG_PACKAGES=tinfra

CXXFLAGS = -g -O0 $(shell pkg-config $(PKG_CONFIG_PACKAGES) --cflags)
LDLIBS = -g $(shell pkg-config $(PKG_CONFIG_PACKAGES) --libs)
CC=g++

invoke_bridge: invoke_bridge.o

clean:
	rm -rf invoke_bridge *.o

