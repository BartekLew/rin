#include "events.h"
#include "common.h"

#include <fblib.h>


void calibrated_point (Context *ctx) {
	if (ctx->last.x - ctx->point.x < Calib(ctx)->threshold.x
		&& ctx->last.y - ctx->point.y < Calib(ctx)->threshold.y)

		return;

	float	x = ((float)ctx->point.x - Calib(ctx)->min.x) / Calib(ctx)->left_right;
	float	y = ((float)ctx->point.y - Calib(ctx)->min.y) / Calib(ctx)->up_down;

	if (x > 0.9 && y > 0.9)
		ctx->point_handler = NULL; // quit

	dot_rgb (ctx->screen, x * ctx->screen.width, y * ctx->screen.height,
			127, 255, 0
	);

	ctx->last = ctx->point;
}

int fb_main (Screen screen, Strings args) {
	if (args.count != 2)
		return 1;

	if (!event_app (args.vals[1], (Application) {
		.point		= &calibrated_point,
		.screen		= screen
	}))
		return 2;

	return 0;
}
