# Makefile
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

CFLAGS= -g -Wall -pedantic
CC=gcc

heatSim: main.c matrix2d.c
	$(CC) $(CFLAGS) -o heatSim main.c matrix2d.c

clean:
	rm -f *.o heatSim

run:	heatSim
	./heatSim 5 10.0 10.0 0.0 0.0 10
