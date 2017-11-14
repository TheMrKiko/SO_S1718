# Makefile
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

CFLAGS= -g -Wall -pedantic
CC=gcc

all: heatSim

heatSim: matrix2d.o main.o
	$(CC) $(CFLAGS) -pthread -o heatSim main.o matrix2d.o

%.o: %.c
	@echo
	@echo "Compiling" $<
	$(CC) $(CFLAGS) -c $<

clean_o:
	rm -f *.o

clean: clean_o
	rm -f heatSim

zip:
	zip G30_entrega3_SO.zip main.c matrix2d.c matrix2d.h Makefile

run:
	@echo
	@echo "Running"
	./heatSim 5 10.0 10.0 0.0 0.0 100 5 0.2

leak:	heatSim
	valgrind --tool=memcheck --leak-check=yes ./heatSim 5 10.0 10.0 0.0 0.0 100 5 0.2
