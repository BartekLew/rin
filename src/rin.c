#include "events.h"
#include "interface.h"
#include "common.h"

#include <fblib.h>
#include <time.h>


void calibrated_point (Context *ctx) {
	if (ctx->last.x - ctx->point.x < Calib(ctx)->threshold.x
		&& ctx->last.y - ctx->point.y < Calib(ctx)->threshold.y)

		return;

	float	x = get_x(ctx);
	float	y = get_y(ctx);

	on_point (ctx->screen, (Point){
		.x = x * ctx->screen.width,
		.y = y * ctx->screen.height
	});

	ctx->last = ctx->point;
}

int fb_main (Screen screen, Strings args) {
	if (args.count != 2)
		return 1;

	listen_commands(&screen);

	if (!event_app (args.vals[1], (Application) {
		.point		= &calibrated_point,
		.screen		= screen
	}))
		return 2;

	return 0;
}
