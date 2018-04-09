#include "events.h"
#include "common.h"


void calibrated_point (Context *ctx) {
	if (ctx->last.x - ctx->point.x < ctx->threshold.x
		&& ctx->last.y - ctx->point.y < ctx->threshold.y )

		return;

	float	x = ((float)ctx->point.x - ctx->min.x) / ctx->up_down;
	float	y = ((float)ctx->point.y - ctx->min.y) / ctx->left_right;
	printf ("\t%f\tx\t%f\r", x, y);

	ctx->last = ctx->point;
}

int main (int argc, char **argv) {
	if (argc != 2)
		return 1;

	if (!event_app (argv[1], (Application) {
		.point		= &calibrated_point
	}))
		return 2;

	return 0;
}
