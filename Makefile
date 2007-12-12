

#PFLAGS=-pg -ftest-coverage -fprofile-arcs
CXXFLAGS=-I. -g $(PFLAGS)
LIBS=-lexpat -rdynamic


all: taskmonitor unittests

TINFRA_OBJECTS=tinfra/tinfra.o tinfra/Symbol.o tinfra/exception.o

taskmonitor: taskmonitor.o $(TINFRA_OBJECTS)
	$(CXX) -g $(PFLAGS) -o $@ $^ $(LIBS)

unittests: test_multitype_map.o test_exception.o unittests.o $(TINFRA_OBJECTS) 
	$(CXX) -g $(PFLAGS) -o $@ $^ $(LIBS) -lunittest++

clean:
	rm -rf taskmonitor.exe test_multitype_map.exe
	find . -name "*.deps" | xargs rm -rf
	find . -name "*.o" | xargs rm -rf
	find . -name "*.gcov" | xargs rm -rf
	find . -name "*.gcda" | xargs rm -rf

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MD -c -o $@ $<
	@[ -d `dirname $<`/.deps ] || mkdir -p `dirname $<`/.deps
	@mv `dirname $<`/*.d `dirname $<`/.deps

-include .deps/*.d
-include tinfra/.deps/*.d
