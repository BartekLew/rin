#include "events.h"
#include "shape_db.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define Letter_db ".letters.dat"

static Mov mov (Point a, Point b) {
	return (Mov) {
		.x = (sint)b.x - a.x,
		.y = (sint)b.y - a.y
	};
}

static uint len (Mov v) {
	return (uint)(sqrt(v.x*v.x + v.y*v.y));
}

static void touch (Context *ctx);
static void move (Context *ctx);
static void release (Context *ctx);

uint	mov_len;
#define Min_curve 100
#define Min_mov   30
 
#define	Route_max 0x20
Point   route[Route_max];
uint	route_len = 0;

static void touch (Context *ctx) {
	mov_len = 0;
	route_len = 1;
	route[0] = ctx->point;
}

static void scale_route (Point *pts, uint len) {
	Point copy[len];
	loop(i, len) copy[i] = pts[i];

	loop(i, len) {
		uint x = 0, y = 0;

		loop(j, len) {
			if (copy[i].x > copy[j].x) x++;
			if (copy[i].y > copy[j].y) y++;
		}
		pts[i] = (Point){
			.x = 10 * x / len,
			.y = 10 * y / len
		};
	}
}

static void move (Context *ctx) {
	uint last_len = len(mov(ctx->last, ctx->point));
	if (last_len < Min_mov)
		return;

	mov_len += last_len;
	uint abs_len = len (mov(route[route_len-1], ctx->point));

	if (abs(mov_len - abs_len) > Min_curve) {
		if (route_len == Route_max) {
			Warn ("Too long route");
			return;
		}
		route[route_len++] = ctx->point;
		mov_len = 0;
	}
}

static void release (Context *ctx) {
	if (mov_len > 0)
		route[route_len++] = ctx->point;

	scale_route (route, route_len);
	loop(i, route_len)
		printf ("%u %u\t", _u route[i].x, _u route[i].y);
	printf("\n");

	Shape *s = shape(route, route_len);
	if (shape_preasent (*s)) {
		printf ("That's %c\n", s->meaning);
	} else {
		printf ("I don't know that one, what's that?> ");
		char c;
		scanf ("%c", &c);
		while (1) {
			char c = fgetc(stdin);
			if (c == '\n') break;
			if (c == '!') {
				store_shapes(Letter_db);
				exit(1);
			}
		}
		s->meaning = c;
	}
}

static void load_db(Context *ctx) {
	load_shapes(Letter_db);
}

int main (int args_count, char **args) {
	if (args_count != 2)
		return 1;

	if (!event_app (args[1], (Application) {
		.init = &load_db,
		.touch = &touch,
		.move = &move,
		.release = &release
	}))
		return 2;

	return 0;
}
