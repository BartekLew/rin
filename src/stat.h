#ifndef STAT_H
#define STAT_H

#include "common.h"

typedef struct {
	sint min, max, sum;
	uint count;
} Stat;

#define New_stat (Stat){0}
void to_stat (Stat *s, sint val);
void print_stat (Stat s, char *label);

#endif
