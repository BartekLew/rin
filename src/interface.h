#ifndef __H_INTERFACE
#define __H_INTERFACE

#include "common.h"

typedef struct {
	u8	r, g, b;
} SColor;

static inline SColor serialize_color (Color c) {
	return (SColor) {
		.r = c.r, .g = c.g, .b = c.b
	};
}

static inline Color restore_color (SColor c) {
	return (Color) {
		.r = c.r, .g = c.g, .b = c.b
	};
}

typedef struct {
	u16	x, y;
	u32	time;
	SColor	color;
	u16	size;
} SerializedPoint;

void on_point (Screen s, Point p);
void listen_commands(Screen *s);

// Get time in 1/100s resolution
uint get_time (void);

#endif 
