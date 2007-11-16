

CXXFLAGS=-I. -g
LIBS=-lexpat
all: taskmonitor

taskmonitor: tinfra/Symbol.o taskmonitor.o
	$(CXX) -g -o $@ $^ $(LIBS)

clean:
	rm -rf taskmonitor.exe *.o

taskmonitor.o: taskmonitor.cpp tinfra/tinfra.h tinfra/tinfra_lex.h tinfra/Symbol.h tinfra/XMLPrinter.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
