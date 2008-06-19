CFLAGS=-O0 -g -I$(HOME)/include
CXXFLAGS=-O0 -g -I$(HOME)/include
LDFLAGS=-g -L$(HOME)/lib -ltinfra

reactor: reactor.o
	g++ $< $(LDFLAGS) -o $@
	
clean:
	rm -rf *.o reactor