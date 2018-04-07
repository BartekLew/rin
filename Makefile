CC=gcc 
LIBS=-lrt
CFLAGS=-Wall -pedantic -std=c99 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=2
TYPE=RELEASE


ifeq ($(TYPE), DEBUG)
	CFLAGS+=-g
endif


all: head bin/rin

rebuild: clean head bin/rin

head:
	@mkdir -p bin/
	@mkdir -p o/

	@echo TYPE	= ${TYPE}
	@echo CC	= ${CC}
	@echo CFLAGS	= ${CFLAGS}
	@echo LIBS	= ${LIBS}
	@echo

bin/rin: src/rin.c o/each_time.o o/events.o 
	@echo "	LNK	$@"
	@${CC} ${CFLAGS} $^ -D${TYPE} -o $@ ${LIBS}
	@echo

o/%.o: src/%.c
	@echo "	CC	$@"
	@${CC} ${CFLAGS} -c $< -D${TYPE} -o $@


clean:
	rm bin/* o/* || true

.PHONY: all head clean tail
