#ifndef __H_INTERFACE
#define __H_INTERFACE

/* Enable framebuffer output code */
#define USE_FB 1

#include "common.h"

typedef struct {
	float	x, y;
	uint	time;
	Color   color;
	uint	size;
} SerializedPoint;

void on_point (Screen s, Point p);
void listen_commands(Screen *s);

// Get time in 1/100s resolution
uint get_time (void);

#endif 
