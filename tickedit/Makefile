CPPFLAGS=`wx-config --cxxflags`
LDLIBS=`wx-config --libs stc` -ltinfra

PROG=tickedit
CC=g++

$(PROG) : tickedit.o

clean:
	rm -rf $(PROG) *.o
