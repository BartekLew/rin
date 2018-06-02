#include "interface.h"

#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define sub_or_zero(A,B) ((A > B) ? A - B : 0)
#define add_or_max(A,B,Max) ((A+B < Max) ? A+B : Max)

#define around_square(S, X, Y, R) \
	for (uint y = sub_or_zero (Y,R); y <= add_or_max (Y,R,S.height); y++)\
		for (uint x = sub_or_zero(X,R); x <= add_or_max (X,R,S.width); x++)

void blur_point (Screen s, Point p, uint r, Color c) {
	around_square (s, p.x, p.y, r) {
		float	dx = (float)x - (float)p.x,
			dy = (float)y - (float)p.y;
		float	ar = sqrtf(dx*dx + dy*dy);

		if (ar <= r) {
			float	filling = ar/r;

			Color	found = get_rgb(s, x, y);
			float	r = (c.r * filling + found.r / filling)/2,
				g = (c.g * filling + found.g / filling)/2,
				b = (c.b * filling + found.b / filling)/2;

			dot_rgb (s, x, y, (r < 255)?r:255, (g<255)?r:255,
					(b < 255)?b:255 );
		}
	}
}

Color brush_color = { .r = 25, .g = 142, .b = 100 };

void on_point (Screen s, Point p) {
	blur_point (s, p, 20, brush_color );
}

uint get_time (void) {
	static struct timespec base = {0};

	struct timespec t;
	clock_gettime (CLOCK_REALTIME, &t);

	if (base.tv_sec == 0 && base.tv_nsec == 0) {
		base = t;
		return 0;
	} else {
		return (t.tv_sec - base.tv_sec) * 100
			+ (t.tv_nsec - base.tv_nsec) / 10000000;
	}
}

pthread_t console;
#define Cmd_max 80

struct ct {
	Color mask;
	char  token;
} color_tokens[] = {
	{.token = 'r', .mask = {.r = 0xff}},
	{.token = 'g', .mask = {.g = 0xff}},
	{.token = 'b', .mask = {.b = 0xff}},
	{.token = 'x', .mask = {.r = 0xff, .g = 0xff, .b = 0xff}}
};

#define ct_size sizeof(struct ct)

void *command_loop (void *ctx) {
	UNUSED(ctx);
	
	char cmd[Cmd_max];
	while (fgets (cmd, Cmd_max-1, stdin) != NULL) {
		for (uint i = 0; i < sizeof(color_tokens)/ct_size; i++) {
			if (color_tokens[i].token == cmd[0]) {
				Color mask = color_tokens[i].mask;
				unsigned int val;

				if (sscanf (cmd+1, "%x", &val) != 1)
					Warn ("wrong parameter: %s", cmd)
				else {
					if (mask.r != 0x00)
						brush_color.r = val;
					if (mask.g != 0x00)
						brush_color.g = val;
					if (mask.b != 0x00)
						brush_color.b = val;
				}
			}
		}
	}

	return NULL;
}

void listen_commands(void) {
	if (pthread_create (&console, NULL, &command_loop, NULL) != 0)
		Die ("Unable to spawn thread");
}

