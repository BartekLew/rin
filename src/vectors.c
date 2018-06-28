#include "events.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	s32 x, y;
} Vector;

Vector total_vector;

#define Max_points 30
Vector	points[Max_points] = {0};
size_t	points_cnt = 0;

void add_point (Vector v) {
	if (abs(v.x) < 15 || abs(v.y) < 15)
		return;

	if (++points_cnt >= Max_points)
		Die ("too many points!");

	points[points_cnt] = (Vector) {
		.x = points[points_cnt-1].x + v.x,
		.y = points[points_cnt-1].y + v.y
	};
}

void print_vector(Vector v) {
	if (abs(v.x) > 15 || abs(v.y) > 15)
		printf ("%d %d\n", v.x, v.y); 
}

/* returns direction value so that it takes values <-20;20>,
   0 -> horizontal, +/-20 -> vertical, +/-10 -> 45deg
   sign respective to sign of tan function. */
s32 direction (Vector v) {
	if (v.x == 0 && v.y == 0) return 0;
	if (v.x < 0) {
		v.x = -v.x;
		v.y = -v.y;
	}

	if (v.y > v.x * 10) return 20;
	if (v.y < v.x * -10) return -20;
	s32 tg = 10 * v.y / v.x;
	if (abs(tg) < 10)
		return tg;
	else if (tg > 0)
		return 10 + tg/10;
	else
		return -10 + tg/10;
}

bool same_direction (Vector a, Vector b) {
	if (a.x * b.x < 0) {
		s32 dira = direction(a);
		if (dira < 0) dira += 40;
		s32 dirb = direction(b);
		if (dirb < 0) dira += 40;

		Testify ("dira=%d dirb=%d\n", dira, dirb);

		return abs (dira - dirb) < 10;
	} else {
		Testify ("da=%d db=%d\n", direction(a), direction(b));
		return abs (direction(a)-direction(b)) < 10;
	}
}

static void touch (Context *ctx);
static void first_move (Context *ctx);
static void next_move (Context *ctx);
static void release (Context *ctx);

uint events;
Vector avg_ev;

static void touch (Context *ctx) {
	events = 2;	
}

static void first_move (Context *ctx) {
	total_vector = (Vector){
		.x = ctx->point.x - ctx->last.x,
		.y = ctx->point.y - ctx->last.y
	};
	avg_ev.x = abs(total_vector.x);
	avg_ev.y = abs(total_vector.y);

	ctx->app.move = &next_move;
}

static void next_move (Context *ctx) {
	Vector v = {
		.x = ctx->point.x - ctx->last.x,
		.y = ctx->point.y - ctx->last.y
	};

	if (!same_direction(v, total_vector)) {
		printf ("** %d %d - %d %d\n", v.x, v.y, total_vector.x, total_vector.y);
		add_point(total_vector);
		total_vector = v;
	} else {
		total_vector.x += v.x;
		total_vector.y += v.y;
	}

	avg_ev.x = (events*avg_ev.x + abs(v.x)) / (events+1);
	avg_ev.y = (events*avg_ev.y + abs(v.y)) / (events+1);
	events++;
}

void release (Context *ctx) {
	add_point(total_vector);
	events++;

	printf("%u events\n", _u events);
	printf("average move: %u %u\n", avg_ev.x, avg_ev.y);

	for (size_t i = 0; i <= points_cnt; i++) {
		s32 new_x = 0, new_y = 0;
		for (size_t j = 0; j <= points_cnt; j++) {
			if (i!=j) {
				if (points[i].x - points[j].x > 20) new_x++;
				if (points[i].y - points[j].y > 20) new_y++;
			}
		}
		printf ("%d %d\n", new_x, new_y);
	}
	ctx->app.move = &first_move;
}

int main (int args_count, char **args) {
	if (args_count != 2)
		return 1;

	if (!event_app (args[1], (Application) {
		.touch = &touch,
		.move = &first_move,
		.release = &release
	}))
		return 2;

	return 0;
}
