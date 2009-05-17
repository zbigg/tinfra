LDLIBS=$(shell pkg-config tinfra --libs) $(shell wx-config --libs)
CXXFLAGS=$(shell pkg-config tinfra --cflags) $(shell wx-config --cxxflags)

test_gui: test_gui.o context.o

