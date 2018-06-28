#include "events.h"

typedef void (*EventHandler) (Event *in, Context *ctx);

static void sync_action (Event *in, Context *ctx);
static void set_point (Event *in, Context *ctx);
static void ignore (Event *ev, Context *ctx);

EventHandler handlers[] = {
	[EV_SYN] = sync_action,
	[EV_ABS] = set_point,
	[EV_KEY] = ignore
};
#define Handlers_Cnt sizeof(handlers) / sizeof(EventHandler)


void event_loop (const int fd, Context *ctx) {
	Event	e;

	while (read (fd, &e, sizeof(e)) == sizeof(e)) {

		if (e.type < Handlers_Cnt && handlers[e.type] != NULL)
			handlers[e.type](&e, ctx);
		else Unsupported(&e);
	}
}

bool event_app (const char *dev, Application app) {
	int fd = open (dev, O_RDONLY);
	if (fd < 0)
		return false;

	Context ctx = {
		.app = app,
		.touching = false
	};

	Testify ("CTX: min=%ux%u max=%ux%u treshold=%ux%u\n",
		_u Calib(&ctx)->min.x, _u Calib(&ctx)->min.y,
		_u Calib(&ctx)->max.x, _u Calib(&ctx)->max.y,
		_u Calib(&ctx)->threshold.x, _u Calib(&ctx)->threshold.y
	);

	if (app.init != NULL)
		app.init (&ctx);

	
	event_loop (fd, &ctx);

	if (app.conclusion != NULL)
		app.conclusion (&ctx);

	close(fd);
	return true;
}

#define detail(Ev_Code, Ctx_Flag, Ctx_Field) \
	if (in->code == Ev_Code) { \
		Testify (#Ctx_Flag ": %d\n", in->value); \
		Protocol_Assumption ( #Ctx_Flag "happens once", \
			(ctx->completeness & Ctx_Flag ) == 0 \
		); \
		ctx->completeness |= Ctx_Flag; \
		\
		ctx->Ctx_Field = in->value; \
	}

	static void set_point (Event *in, Context *ctx) {
		detail (ABS_X, CTX_X, point.x)
		else detail (ABS_Y, CTX_Y, point.y)
		else detail (ABS_PRESSURE, CTX_PRESSURE, pressure)
		else Unsupported(in);
	}

#undef detail

static void ignore (Event *ev, Context *ctx) {
	UNUSED(ev);
	UNUSED(ctx);
}

static void sync_action (Event *in, Context *ctx) {
	if (ctx->completeness & CTX_TOUCH) {
		if (ctx->pressure > 0) {
			if (!ctx->touching) {
				ctx->touching = true;
				if (ctx->app.touch)
					ctx->app.touch(ctx);
			} else {
				if (ctx->app.move)
					ctx->app.move(ctx);
			}
			ctx->last = ctx->point;
		} else {
			ctx->touching = false;
			if (ctx->app.release)
				ctx->app.release(ctx);
		}
	}

	fflush(stdout);
	ctx->completeness = 0;
}

