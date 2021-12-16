all: phil

phil: phil.c
	gcc -Wall -g -o phil phil.c

clean:
	rm -rf *~ *.o phil
