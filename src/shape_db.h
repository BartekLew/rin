#ifndef SHAPE_DB_H
#define SHAPE_DB_H

#include "common.h"

typedef struct {
	u8	x, y;
} SmallPoint;

typedef struct {
	uint		point_pos, len;
	char		meaning;
} Shape;

static inline bool shape_preasent (Shape s) {
	return s.meaning != 0;
}

Shape *shape(Point *sh, uint len);
void load_shapes(const char *file);
void store_shapes(const char *file);

#endif /* SHAPE_DB_H */
