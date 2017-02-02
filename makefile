CFLAGS=-c -Wall -O2
LIBS = -lm -lpthread

all: bme280

bme280: main.o
	$(CC) $(LIBS) main.o -o bme280

main.o: main.c
	$(CC) $(CFLAGS) main.c

clean:
	rm -rf *o bme280
