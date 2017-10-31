# Makefile
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

CFLAGS= -g -Wall -pedantic
CC=gcc

all: heatSim clean_o

heatSim: matrix2d.o mplib3.o leQueue.o main.o
	$(CC) $(CFLAGS) -lpthread -o heatSim main.o matrix2d.o mplib3.o leQueue.o

%.o: %.c
	@echo
	@echo "Compiling" $<
	$(CC) $(CFLAGS) -c $<

clean_o:
	rm -f *.o

clean: clean_o
	rm -f heatSim

zip:
	zip 86416_testepratico1_SO.zip main.c matrix2d.c mplib3.c leQueue.c matrix2d.h mplib3.h leQueue.h Makefile README.md

run:    heatSim clean_o
	@echo
	@echo "Running"
	./heatSim 5 10.0 10.0 0.0 0.0 10 5 0

leak:	heatSim
	valgrind --tool=memcheck --leak-check=yes ./heatSim 5 10.0 10.0 0.0 0.0 10 1 0

.PHONY:
	clean leak zip
