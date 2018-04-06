CC=gcc -Wall -pedantic -std=c99
TYPE=RELEASE

all: head bin/rin

rebuild: clean head bin/rin

head:
	@mkdir -p bin/
	@mkdir -p o/

	@echo CC = ${CC}
	@echo


bin/rin: src/rin.c o/events.o
	${CC} $^ -D${TYPE} -o $@

o/%.o: src/%.c
	${CC} -c $< -D${TYPE} -o $@


clean:
	rm bin/* o/* || true

.PHONY: all head clean
