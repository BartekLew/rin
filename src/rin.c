#include "events.h"
#include "common.h"

#include <time.h>

void calibrated_point (Context *ctx) {
	if (ctx->last.x - ctx->point.x < Calib(ctx)->threshold.x
		&& ctx->last.y - ctx->point.y < Calib(ctx)->threshold.y)

		return;

	float	x = get_x(ctx);
	float	y = get_y(ctx);

    printf(":%f x %fx\n", x, y);

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
