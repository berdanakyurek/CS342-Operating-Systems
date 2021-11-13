all: schedule

schedule: schedule.c
	gcc -Wall -g -o schedule schedule.c -lm

clean:
	rm -rf *~ *.o schedule
