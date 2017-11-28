CFLAGS=-c -Wall -O2
LIBS = -lm -lpthread

all: libbme280.a

libbme280.a: bme280.o
	ar -rc libbme280.a bme280.o ;\
	sudo cp libbme280.a /usr/local/lib ;\
	sudo cp bme280.h /usr/local/include

bme280.o: bme280.c
	$(CC) $(CFLAGS) bme280.c

clean:
	rm *.o libbme280.a
