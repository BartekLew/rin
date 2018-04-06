CC=gcc -Wall -pedantic -std=c99

all: head bin/rin.dbg bin/rin


head:
	@mkdir -p bin/
	@echo CC = ${CC}
	@echo


bin/rin.dbg: rin.c
	${CC} -DDEBUG $^ -o $@

bin/rin: rin.c
	${CC} $^ -o $@

clean:
	rm bin/*

.PHONY: all head clean
