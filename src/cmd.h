#ifndef CMD_H
#define CMD_H

#include "common.h"
#include "shape_db.h"

typedef struct {
	char *tok;
	void (*action)(void);
} Command;

void listen_commands(void);
void push_shape(Shape*);

#endif
