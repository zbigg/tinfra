CPPFLAGS=`wx-config --cxxflags` -g
LDLIBS=`wx-config --libs stc` -ltinfra -g

PROG=tickedit
CC=g++

$(PROG) : tickedit.o

clean:
	rm -rf $(PROG) *.o
