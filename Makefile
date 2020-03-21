CC = gcc
CFLAGS = -g -Wall
LDFLAGS =  -lrt -lpigpio
.PHONY: all
all: main.c sps30_i2c_lib.o
	$(CC) $(CFLAGS) -o main.o main.c sps30_i2c_lib.o $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o *.out

