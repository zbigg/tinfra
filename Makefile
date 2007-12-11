

#PFLAGS=-pg -ftest-coverage -fprofile-arcs
CXXFLAGS=-I. -g $(PFLAGS)
LIBS=-lexpat 
LDFLAGS=$(LIBS)

#-rdynamic
LD=g++

all: taskmonitor unittests

TINFRA_OBJECTS=tinfra/tinfra.o tinfra/Symbol.o tinfra/exception.o tinfra/xml/XMLStream.o tinfra/xml/XMLStreamReader.o

taskmonitor: taskmonitor.o $(TINFRA_OBJECTS)
	$(CXX) -g $(PFLAGS) -o $@ $^ $(LIBS)

unittests: test_multitype_map.o unittests.o $(TINFRA_OBJECTS) 
	$(CXX) -g $(PFLAGS) -o $@ $^ $(LIBS) -lunittest++

clean:
	rm -rf taskmonitor.exe test_multitype_map.exe
	find . -name ".deps" | xargs rm -rf
	find . -name "*.o" | xargs rm -rf
	find . -name "*.gcov" | xargs rm -rf
	find . -name "*.gcda" | xargs rm -rf

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MD -c -o $@ $<
	@[ -d `dirname $<`/.deps ] || mkdir -p `dirname $<`/.deps
	@mv `dirname $<`/*.d `dirname $<`/.deps

-include .deps/*.d
-include tinfra/.deps/*.d
