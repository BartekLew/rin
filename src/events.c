#include "events.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef void (*EventHandler) (Event *in, Context *ctx);

void sync_action (Event *in, Context *ctx);
void set_point (Event *in, Context *ctx);
void ignore (Event *ev, Context *ctx);

EventHandler handlers[] = {
	[EV_SYN] = sync_action,
	[EV_ABS] = set_point,
	[EV_KEY] = ignore
};
#define Handlers_Cnt sizeof(handlers) / sizeof(EventHandler)


bool event_loop (const char *dev, Handler h) {
	int fd = open (dev, O_RDONLY);
	if (fd < 0)
		return false;

	Event	e;
	Context	ctx = { .point_handler = h };
	while ( ctx.point_handler != NULL
		&& read (fd, &e, sizeof(e)) == sizeof(e)) {

		if (e.type < Handlers_Cnt && handlers[e.type] != NULL)
			handlers[e.type](&e, &ctx);
		else Unsupported(&e);
	}

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

	void set_point (Event *in, Context *ctx) {
		detail (ABS_X, CTX_X, x)
		else detail (ABS_Y, CTX_Y, y)
		else detail (ABS_PRESSURE, CTX_PRESSURE, pressure)
		else Unsupported(in);
	}

#undef detail

void ignore (Event *ev, Context *ctx) {
	UNUSED(ev);
	UNUSED(ctx);
}

void sync_action (Event *in, Context *ctx) {

	if (ctx->completeness & CTX_TOUCH) {
		/*
			Not all fields are mandatory to happen, if not
			assume value didn't change.
		*/

		ctx->point_handler(ctx);
	}

	fflush(stdout);
	ctx->completeness = 0;

}

