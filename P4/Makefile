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
	rm -f *.o teste_temp.txt

clean: clean_o
	rm -f heatSim

zip:
	zip G30_entrega4_SO.zip main.c matrix2d.c matrix2d.h Makefile

run: heatSim
	@echo
	@echo "Running"
	./heatSim 1000 10.0 10.0 0.0 0.0 2000 5 0 teste_temp.txt 1

leak:	heatSim
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./heatSim 1000 10.0 10.0 0.0 0.0 2000 5 0 teste_temp.txt 1
