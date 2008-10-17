LDLIBS=-ltinfra -lpthread
CC=g++

sftp: sftp.o sftp_protocol.o

sftp_protocol.h: rfc4251.h
sftp_protocol.o: sftp_protocol.h
sftp.o: sftp_protocol.h
