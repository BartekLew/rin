#include "shape_db.h"
#include <math.h>

typedef struct {
	uint points, shapes;
} DbHeader;

#define Max_points 0x10000
SmallPoint points[Max_points];
uint applied_points = 0, points_cnt = 0;

#define Max_shapes 0x1000
Shape shapes[Max_points];
uint applied_shapes = 0, shapes_cnt = 0;

static bool shape_match (Point *shape, uint len, Shape patt) {
	if (len != patt.len)
		return false;

	loop (i, len-1) {
		if (shape[i].x != (points+patt.point_pos)[i].x)
			return false;

		if (shape[i].y != (points+patt.point_pos)[i].y)
			return false;
	}

	return true;
}

Shape *shape(Point *sh, uint len) {
	loop (i, applied_shapes) {
		if (shape_match (sh, len, shapes[i]))
			return shapes+i;
	}

	if (points_cnt + len > Max_points
		|| shapes_cnt + 1 > Max_shapes) {
		Die ("No more room in shapes db.");
	}
		
	loop (i, len) points[i+points_cnt] = (SmallPoint) {
			.x = sh[i].x,
			.y = sh[i].y
		};
	Shape *result = shapes + shapes_cnt++;
	result->point_pos = points_cnt;
	result->len = len;
	points_cnt += len;
	return result;
}

void load_shapes (const char *file) {
	optional_file (file, Read, in) {
		DbHeader h;
		read_one (h, in);
		applied_points = points_cnt = h.points;
		applied_shapes = shapes_cnt = h.shapes;

		if (!read_exact(points, points_cnt, in)
			|| !read_exact(shapes, shapes_cnt, in)) {
			Die ("Malformed shapedb file: %s\n", file);
		}

		printf ("Loaded letter db, %u/%u %u bytes\n",
			_u points_cnt, _u shapes_cnt, _u ftell(in)
		);
	}
}

void store_shapes (const char *file) {
	optional_file (file, Write, out) {
		DbHeader h = {
			.shapes = applied_shapes,
			.points = applied_points
		};
		
		write_one_w (h, out, file);
		write_exact_w (points, applied_points, out, file);
		write_exact_w (shapes, applied_shapes, out, file);

		printf ("Stored letter db, %u/%u %u bytes\n",
			_u points_cnt, _u shapes_cnt, _u ftell(out)
		);
	}
}

void iterate_letter (char letter, LetterIteration action) {
	loop (i, applied_shapes) {
		if (shapes[i].meaning == letter)
			action (points + shapes[i].point_pos, shapes[i].len);
	}
}

void drop_changes(void) {
	points_cnt = applied_points;
	shapes_cnt = applied_shapes;
}

void apply_changes(void) {
	applied_points = points_cnt;
	applied_shapes = shapes_cnt;
}
