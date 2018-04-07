#include "events.h"
#include "common.h"


void sync_action (Event *in, Context *ctx);

Handler handlers[] = {
	[EV_SYN] = sync_action,
	[EV_ABS] = set_point,
	[EV_KEY] = ignore
};
#define Handlers_Cnt sizeof(handlers) / sizeof(Handler)


int main (int argc, char **argv) {
	if (argc != 2)
		return 1;

	if (!event_loop (argv[1], handlers, Handlers_Cnt))
		return 2;

	return 0;
}

void sync_action (Event *in, Context *ctx) {

	#ifndef TESTIFY

	if (ctx->completeness & CTX_TOUCH) {
		/*
			Not all fields are mandatory to happen, if not
			assume value didn't change.
		*/

		printf( "cur @ %6u,%6u %6u %6u\r",
			ctx->x, ctx->y, ctx->pressure, ctx->id
		);
	}

	#else

	printf( "\n" );

	#endif

	fflush(stdout);
	ctx->completeness = 0;

}

