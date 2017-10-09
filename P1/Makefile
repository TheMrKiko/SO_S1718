# Makefile
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

CFLAGS2= -g -Wall -pedantic
CFLAGS= -g -Wall
CC=gcc

all: heatSim

heatSim: main.c matrix2d.c mplib3.c leQueue.c matrix2d.h mplib3.h leQueue.h
	$(CC) $(CFLAGS) -lpthread -o heatSim main.c matrix2d.c mplib3.c leQueue.c

clean:
	rm -f heatSim

zip:
	zip heatSim_ex01_solucao.zip main.c matrix2d.c matrix2d.h Makefile

run:    heatSim
	./heatSim 5 10.0 10.0 0.0 0.0 10 5 2
	
run1:
	./heatSim 5 10.0 10.0 0.0 0.0 10 1 2

leak:	heatSim
	valgrind --tool=memcheck --leak-check=yes ./heatSim 5 10.0 10.0 0.0 0.0 10 5 2
