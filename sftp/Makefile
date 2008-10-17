LDLIBS=-ltinfra -lwsock32 -lpthread
CC=g++

sftp: sftp.o sftp_protocol.o

sftp_protocol.o: sftp_protocol.h
sftp.o: sftp_protocol.h
