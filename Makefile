LDLIBS= --enable-auto-import $(shell pkg-config tinfra --libs) $(shell wx-config --libs)
CXXFLAGS=$(shell pkg-config tinfra --cflags) $(shell wx-config --cxxflags)
CC=g++
PROGRAMS=test_gui

test_gui: test_gui.o context.o render.o controls/AdvDialog.o

clean:
	rm -rf *.o $(PROGRAMS)


