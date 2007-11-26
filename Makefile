

PFLAGS=-pg -ftest-coverage -fprofile-arcs
CXXFLAGS=-I. -g $(PFLAGS) -I/c/Programs/PostgreSQL_82/include 
LIBS=-lexpat -L/c/Programs/PostgreSQL_82/lib -lpq
all: taskmonitor

taskmonitor: tinfra/Symbol.o taskmonitor.o
	$(CXX) -g $(PFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf taskmonitor.exe *.o tinfra/*.o

taskmonitor.o: taskmonitor.cpp tinfra/tinfra.h tinfra/tinfra_lex.h tinfra/Symbol.h tinfra/XMLPrinter.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
