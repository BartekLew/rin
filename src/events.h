#ifndef __H_EVENTS
#define __H_EVENTS

#include "common.h"

#include <linux/input.h>

typedef struct {
	uint	x, y, pressure;
} Point;
typedef void (*Handler) (const Point *p);

bool event_loop (const char *dev, Handler point_handler);

#endif
