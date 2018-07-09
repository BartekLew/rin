#include "shape_db.h"
#include <math.h>

typedef struct {
	uint points, shapes;
} DbHeader;

#define Max_points 0x1000
SmallPoint points[Max_points];
uint points_cnt = 0;

#define Max_shapes 0x100
Shape shapes[Max_points];
uint shapes_cnt = 0;

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
	loop (i, shapes_cnt) {
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
		points_cnt = h.points;
		shapes_cnt = h.shapes;

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
			.shapes = shapes_cnt,
			.points = points_cnt
		};
		
		write_one_w (h, out, file);
		write_exact_w (points, points_cnt, out, file);
		write_exact_w (shapes, shapes_cnt, out, file);

		printf ("Stored letter db, %u/%u %u bytes\n",
			_u points_cnt, _u shapes_cnt, _u ftell(out)
		);
	}
}
