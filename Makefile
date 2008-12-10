TINFRA_SRC=/home/zbigg/projects/tinfra/trunk

CXXFLAGS=-O0 -g -Wall -Werror -pedantic -I$(TINFRA_SRC)
LDLIBS=-L$(HOME)/lib -g -ltinfra -lpthread 
CC=g++

LANG=C
export LANG

interruptible2: interruptible2.o

posix_signals: posix_signals.o


clean:
	rm -rf *.o ftt