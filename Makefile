

PFLAGS=-pg -ftest-coverage -fprofile-arcs
CXXFLAGS=-I. -g $(PFLAGS) -I`pg_config --includedir`
LIBS=-lexpat -L`pg_config --libdir` -lpq
all: taskmonitor

taskmonitor: tinfra/Symbol.o taskmonitor.o
	$(CXX) -g $(PFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf taskmonitor.exe *.o tinfra/*.o

taskmonitor.o: taskmonitor.cpp tinfra/tinfra.h tinfra/tinfra_lex.h tinfra/Symbol.h tinfra/XMLPrinter.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
