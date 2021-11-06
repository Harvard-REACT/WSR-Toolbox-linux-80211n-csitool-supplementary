CFLAGS = -O3 -Wall -static -Wno-unknown-pragmas
LDLIBS = -lorcon -lm -lrt
CC = gcc

ALL = random_packets_multiple

all: $(ALL)

clean:
	rm -f *.o $(ALL)

random_packets_multiple: random_packets_multiple.c util.o

util.c: util.h
