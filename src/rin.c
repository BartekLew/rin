#include "events.h"
#include "common.h"

bool first_time = true;

typedef struct {
	uint x, y;
} Point;

typedef struct {
	const char	*label;
	Point		min, max;
} Test;


#define Tests_Cnt 6
Test tests[Tests_Cnt] = {
	{.label = "Up-Left"},
	{.label = "Up-Right"},
	{.label = "Down-Left"},
	{.label = "Down-Right"},
	{.label = "Tap center"},
	{.label = "Tap center again"}
};
uint test_no = 0;

void update_minmax(Point *min, Point *max, Point current) {
	if (first_time) {
		min->x = max->x = current.x;
		min->y = max->y = current.y;
		first_time = false;
	} else {
		if (min->x > current.x) min->x = current.x;
		else if (max->x < current.x) max->x = current.x;
	
		if (max->y < current.y) max->y = current.y;
		else if (min->y > current.y) min->y = current.y;
	}
	
}

void point (Context *ctx) {
	Point *min = &tests[test_no].min;
	Point *max = &tests[test_no].max;

	if (ctx->pressure > 0) {
		update_minmax (min, max, (Point){.x = ctx->x, .y = ctx->y});
	} else {
		++test_no;
		if (test_no == Tests_Cnt)
			ctx->point_handler = NULL;
		else {
			printf( "please touch at %s corner.\n", tests[test_no].label );
			first_time = true;
		}
	}
}

void print_square (const char *label, Point *min, Point *max) {
	printf( "%20s: x = <%u;%u>\ty = <%u;%u>\t%ux%u\n",
		label, min->x, max->x, min->y, max->y,
		max->x - min->x, max->y - min->y
	);
}

int main (int argc, char **argv) {
	if (argc != 2)
		return 1;

	printf( "please touch at %s.\n", tests[0].label );
	if (!event_loop (argv[1], &point))
		return 2;

	Point	min = tests[0].min,
		max = tests[0].max;

	for (uint i = 0; i < Tests_Cnt; i++) {
		print_square (tests[i].label, &tests[i].min, &tests[i].max);
		update_minmax (&min, &max, tests[i].min);
		update_minmax (&min, &max, tests[i].max);
	}

	print_square ("Work space", &min, &max);

	return 0;
}
