all: phil

phil: phil.c
	gcc -Wall -g -o phil phil.c -lpthread

clean:
	rm -rf *~ *.o phil
