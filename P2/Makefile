# Makefile
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

CFLAGS= -g -Wall -pedantic
CC=gcc

all: heatSim

heatSim: main.c matrix2d.c mplib3.c leQueue.c matrix2d.h mplib3.h leQueue.h
	$(CC) $(CFLAGS) -lpthread -o heatSim main.c matrix2d.c mplib3.c leQueue.c

clean:
	rm -f heatim

zip:
	zip heatSim_ex02_solucao.zip main.c matrix2d.c mplib3.c leQueue.c matrix2d.h mplib3.h leQueue.h Makefile

run:    heatSim
	./heatSim 5 10.0 10.0 0.0 0.0 10 1 0

leak:	heatSim
	valgrind --tool=memcheck --leak-check=yes ./heatSim 5 10.0 10.0 0.0 0.0 10 1 0