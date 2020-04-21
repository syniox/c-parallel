CC=gcc

parallel: parallel.c
	${CC} -o parallel parallel.c -lpthread -Wall -O2 -static

.PHONY: clean
clean:
	rm parallel
