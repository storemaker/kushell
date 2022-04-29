kushell: main.o server.o client.o helpers.o
	gcc main.o server.o client.o helpers.o -o kushell

main.o: main.c
	gcc -c main.c

server.o: server.c
	gcc -c server.c

client.o: client.c
	gcc -c client.c

helpers.o: helpers.c
	gcc -c helpers.c

clean:
	rm *.o kushell
