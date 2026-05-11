all: server peer-client

server: server.c protocol.c protocol.h
	gcc -o server server.c protocol.c

peer-client: peer-client.c protocol.c protocol.h
	gcc -o peer-client peer-client.c protocol.c

clean:
	rm -f server peer-client