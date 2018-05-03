#ifndef __H_INTERFACE
#define __H_INTERFACE

#include "common.h"

typedef struct {
	float	x, y;
	uint	t;
} SerializedPoint;

void on_point (Screen s, Point p);

// Get time in 1/100s resolution
uint get_time (void);

#endif 
