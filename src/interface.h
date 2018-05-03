#ifndef __H_INTERFACE
#define __H_INTERFACE

#include "common.h"

typedef struct {
	float	x, y;
} SerializedPoint;

void on_point (Screen s, Point p);

#endif 
