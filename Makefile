all: statclient statserver statclient_th statserver_th

statclient: statclient.c
	gcc -Wall -o statclient statclient.c -lrt

statserver: statserver.c
	gcc -Wall -O2 -o statserver statserver.c -lrt

statclient_th: statclient_th.c
	gcc -Wall -o statclient_th statclient_th.c -lrt

statserver_th: statserver_th.c
	gcc -Wall -O2 -o statserver_th statserver_th.c -lrt -lpthread

clean:
	rm -rf *~ statclient statserver statclient_th statserver_th
