#include "events.h"
#include "common.h"

#include <fblib.h>
#include <math.h>


void blur_point (Screen s, Point p, uint r, Color c) {
	for (uint y = p.y - r; y <= p.y + r; y++) {
		for (uint x = p.x - r; x <= p.x + r; x++) {
			float	dx = (float)x - (float)p.x,
				dy = (float)y - (float)p.y;
			float	ar = sqrtf(dx*dx + dy*dy);

			if (ar <= r) {
				float	filling = ar/r;

				Color	found = get_rgb(s, x, y);
				float	r = (c.r * filling + found.r / filling)/2,
					g = (c.g * filling + found.g / filling)/2,
					b = (c.b * filling + found.b / filling)/2;
	
				dot_rgb (s, x, y, (r < 255)?r:255, (g<255)?r:255,
						  (b < 255)?b:255 );
			}

		}
	}
}

void calibrated_point (Context *ctx) {
	if (ctx->last.x - ctx->point.x < Calib(ctx)->threshold.x
		&& ctx->last.y - ctx->point.y < Calib(ctx)->threshold.y)

		return;

	float	x = ((float)ctx->point.x - Calib(ctx)->min.x) / Calib(ctx)->left_right;
	float	y = ((float)ctx->point.y - Calib(ctx)->min.y) / Calib(ctx)->up_down;

	if (x > 0.9 && y > 0.9)
		ctx->point_handler = NULL; // quit

#if 0
	dot_rgb (ctx->screen, x * ctx->screen.width, y * ctx->screen.height,
			127, 255, 0
	);
#endif
	blur_point (ctx->screen, (Point){ .x = x * ctx->screen.width,
					  .y = y * ctx->screen.height },
		20, (Color) { .r = 25, .g = 142, .b = 100 }
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
