#include "events.h"
#include "interface.h"
#include "common.h"

#include <fblib.h>
#include <time.h>


#ifdef DUMP_POINTS
	#define Dump_touch(X,Y,T) { \
		SerializedPoint pos = {.x = X, .y = Y, .t = T}; \
		fwrite (&pos, sizeof(pos), 1, stdout); \
	}
#else
	#define Dump_touch(X,Y,T)
#endif

void calibrated_point (Context *ctx) {
	if (ctx->last.x - ctx->point.x < Calib(ctx)->threshold.x
		&& ctx->last.y - ctx->point.y < Calib(ctx)->threshold.y)

		return;

	float	x = get_x(ctx);
	float	y = get_y(ctx);

	Dump_touch(x,y,get_time());

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
