#include "common.h"
#include "events.h"
#include "calibration.h"


void event_loop (const int fd, Context *ctx);

bool first_time = true;

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

void testify_square (const char *label, Point *min, Point *max) {
	Testify ("%20s: x = <%u;%u>\ty = <%u;%u>\t%ux%u\n",
		label, min->x, max->x, min->y, max->y,
		max->x - min->x, max->y - min->y
	);
}

Point square_size (Point min, Point max) {
	return (Point) {
		.x = max.x - min.x,
		.y = max.y - min.y
	};
}

Point point_max (Point a, Point b) {
	return (Point) {
		.x = (a.x > b.x)? a.x : b.x,
		.y = (a.y > b.y)? a.y : b.y
	};
}

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


void before_calibration (Context *ctx) {
	printf( "please touch at %s.\n", tests[0].label );
}

void finish_calibration (Context *ctx) {
	Point	min = tests[0].min,
		max = tests[0].max;

	for (uint i = 0; i < Tests_Cnt; i++) {
		testify_square (tests[i].label, &tests[i].min, &tests[i].max);
		update_minmax (&min, &max, tests[i].min);
		update_minmax (&min, &max, tests[i].max);
	}

	testify_square ("Work space", &min, &max);

	Calib(ctx)->min = min;
	Calib(ctx)->max = max;

	Calib(ctx)->threshold = point_max (
		square_size (tests[5].min, tests[5].max),
		square_size (tests[4].min, tests[4].max)
	);
	Testify ("%20s: %ux%u.\n", "Touch size",
		Calib(ctx)->threshold.x, Calib(ctx)->threshold.y
	);

	Calib(ctx)->up_down = (tests[2].min.y > tests[0].max.y)
				? max.y - min.y : min.y - max.y;
	Calib(ctx)->left_right = (tests[1].min.x > tests[0].max.x)
				? max.x - min.x : min.x - max.x;

	Testify ("\t\t  UD: %d ; LR: %d\n",
		Calib(ctx)->up_down, Calib(ctx)->left_right
	);
}

void calibration_point (Context *ctx) {
	Point *min = &tests[test_no].min;
	Point *max = &tests[test_no].max;

	if (ctx->pressure > 0) {
		update_minmax (min, max, (Point){.x = ctx->point.x, .y = ctx->point.y});
	} else {
		++test_no;
		if (test_no == Tests_Cnt)
			ctx->point_handler = NULL;
		else {
			printf( "please touch at %s.\n", tests[test_no].label );
			first_time = true;
		}
	}
}

Calibration get_calibration (int devfd) {
	Calibration result = { .fd = open (Calibration_file, O_RDONLY) };
	bool have_ctx = result.fd > 0;
	if (have_ctx)
		close(result.fd);
	result.fd = open (Calibration_file, O_RDWR|O_CREAT, (mode_t) 00600);
	if (result.fd < 0)
		Die("Cannot open %s for R/W\n", Calibration_file);

	if (!have_ctx) {
		Context c;
		if (write (result.fd, &c, sizeof(Context)) < sizeof(c))
			Die ("Cannot fit ctx structure in %s.\n",
				Calibration_file
			);
		lseek(result.fd, 0, SEEK_SET);
	}

	result.data = mmap (0, sizeof(Context), PROT_READ|PROT_WRITE,
		MAP_SHARED, result.fd, 0
	);
	if (result.data == MAP_FAILED)
		Die ("Mmap %s failed\n", Calibration_file);

	if (!have_ctx) {
		Context c = {
			.point_handler = calibration_point,
			.calibration = result
		};

		before_calibration (&c);
		event_loop (devfd, &c);
		finish_calibration (&c);
	}

	return result;
}

