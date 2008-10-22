LDLIBS=-ltinfra -lpthread -s
CC=g++
CXXFLAGS=-O3

sftp: sftp.o sftp_protocol.o
sftp_protocol.h: rfc4251.h primitive_wrapper.h 
sftp_protocol.o: sftp_protocol.h
sftp.o: sftp_protocol.h

clean:
	rm -rf *.o sftp
