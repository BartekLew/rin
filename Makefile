CC=gcc 
LIBS=-lrt
CFLAGS=-Wall -pedantic -std=c99
INCLUDE=-Ifblib/
TYPE=RELEASE

fblib.a_URL=https://github.com/BartekLew/fblib

ifeq ($(TYPE), DEBUG)
	CFLAGS+=-g
endif


all: head bin/rin

rebuild: clean head bin/rin

head:
	@mkdir -p bin/
	@mkdir -p o/

	@echo "TYPE	= ${TYPE}"
	@echo "CC	= ${CC}"
	@echo "CFLAGS	= ${CFLAGS}"
	@echo "INCLUDE	= ${INCLUDE}"
	@echo "LIBS	= ${LIBS}"
	@echo

bin/rin: src/rin.c o/events.o o/calibration.o fblib/fblib.a
	@echo "	LNK	$@"
	@${CC} ${CFLAGS} ${INCLUDE} $^ -D${TYPE} -o $@ ${LIBS}
	@echo

o/%.o: src/%.c
	@echo "	CC	$@"
	@${CC} ${CFLAGS} ${INCLUDE} -c $< -D${TYPE} -o $@

%.a:
	@echo "Missing dependency: $@. Link it in this directory."
	@echo "look at: ${$@_URL}"
	@echo
	@false

clean:
	rm bin/* o/* || true

.PHONY: all head clean tail
