CC=gcc

parallel: parallel.c
	${CC} -o parallel parallel.c -lpthread -latomic -Wall -O2 -static

.PHONY: clean
clean:
	rm parallel
