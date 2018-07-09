#include "events.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static Mov mov (Point a, Point b) {
	return (Mov) {
		.x = (sint)b.x - a.x,
		.y = (sint)b.y - a.y
	};
}

static uint len (Mov v) {
	return (uint)(sqrt(v.x*v.x + v.y*v.y));
}

static Mov add (Mov a, Mov b) {
	return (Mov) {.x = a.x + b.x, .y = a.y + b.y};
}

static void touch (Context *ctx);
static void move (Context *ctx);
static void release (Context *ctx);

Point	start, ref;
uint	mov_len;
#define Min_curve 100
#define Min_mov   30

static void touch (Context *ctx) {
	ref = start = ctx->point;
	mov_len = 0;
}

static void print_last_mov(Context *ctx){
	printf( "%d %d -> %d %d (%u)\n",
			_s (start.x - ref.x), _s (start.y - ref.y),
			_s (ctx->point.x - ref.x), _s (ctx->point.y - ref.y),
			_u len (mov(start, ctx->point))
	      );
}

static void move (Context *ctx) {
	uint last_len = len(mov(ctx->last, ctx->point));
	if (last_len < Min_mov)
		return;

	mov_len += last_len;
	uint abs_len = len (mov(start, ctx->point));

	if (abs(mov_len - abs_len) > Min_curve) {
		print_last_mov(ctx);
		start = ctx->point;
		mov_len = 0;
	}
}

static void release (Context *ctx) {
	print_last_mov(ctx);
}

int main (int args_count, char **args) {
	if (args_count != 2)
		return 1;

	if (!event_app (args[1], (Application) {
		.touch = &touch,
		.move = &move,
		.release = &release
	}))
		return 2;

	return 0;
}
