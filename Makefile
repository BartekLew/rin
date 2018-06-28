CC=gcc -D_POSIX_C_SOURCE=199309L
LIBS=-lrt -lm -lpthread
CFLAGS=-Wall -pedantic -std=c99
INCLUDE=-Ifblib/
TYPE=RELEASE
OPTS=

fblib.a_URL=https://github.com/BartekLew/fblib

ifeq ($(TYPE), DEBUG)
	CFLAGS+=-g
endif


all: head bin/vectors

rebuild: clean all

head:
	@mkdir -p bin/
	@mkdir -p o/

	@echo "TYPE	= ${TYPE}"
	@echo "CC	= ${CC}"
	@echo "CFLAGS	= ${CFLAGS}"
	@echo "INCLUDE	= ${INCLUDE}"
	@echo "LIBS	= ${LIBS}"
	@echo "OPTS	= ${OPTS}"
	@echo

bin/vectors: src/vectors.c o/events.o
	@echo "	LNK	$@"
	@${CC} ${CFLAGS} ${INCLUDE} $^ -D${TYPE} ${OPTS} -o $@ ${LIBS}

o/%.o: src/%.c
	@echo "	CC	$@"
	@${CC} ${CFLAGS} ${INCLUDE} -c $< -D${TYPE} ${OPTS} -o $@

%.a:
	@echo "Missing dependency: $@. Link it in this directory."
	@echo "look at: ${$@_URL}"
	@echo
	@false

clean:
	rm bin/* o/* || true

.PHONY: all head clean tail
