#include "stat.h"

#include <stdio.h>

void to_stat (Stat *s, sint val) {
	if (s->count++ == 0) {
		s->min = s->max = s->sum = val;
	} else {
		if (s->min > val) s->min = val;
		if (s->max < val) s->max = val;
		s->sum += val;
	}
}

void print_stat (Stat s, char *label) {
	printf ("%s: %d-%d, avg. %f\n", label,
		_s s.min, _u s.max, (float)s.sum / s.count
	);
}
