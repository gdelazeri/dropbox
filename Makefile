CC=g++
INCLUDE=-I./util
FLAGS=-O2 -Wall -pthread -std=gnu++11
CFLAGS=$(FLAGS) $(INCLUDE)

UTIL=util/socket.o util/userServer.o util/request.o util/file.o util/dropboxUtil.o util/device.o

.PHONY: all clean

all: dropboxServer dropboxClient

dropboxServer: $(UTIL) server/dropboxServer.o
	$(CC) $(CFLAGS) -o $@ $^

dropboxClient: $(UTIL) client/dropboxClient.o client/user.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.bak *. ./*/*.o dropboxClient dropboxServer
