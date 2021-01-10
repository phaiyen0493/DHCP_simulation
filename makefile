all: server client

client:client.o
	gcc -o client client.o

client.o: client.c
	gcc -c client.c

server: server.o
	gcc -o server -pthread server.o

server.o: server.c
	gcc -c server.c

clean:
	rm *.o
	rm client
	rm server
