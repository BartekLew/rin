#include "rin.h"
#include "common.h"


void sync (Event *in, Context *ctx) {

	#ifndef TESTIFY

	if (ctx->completeness & CTX_TOUCH) {
		/*
			Not all fields are mandatory to happen, if not
			assume value didn't change.
		*/

		printf( "cur @ %6lu,%6lu %6lu %6lu\n",
			ctx->x, ctx->y, ctx->pressure, ctx->id
		);
	}

	#else

	printf( "\n" );

	#endif

	ctx->completeness = 0;

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
		else detail (ABS_MT_TRACKING_ID, CTX_UNIQ_PRESS, id)
		else Unsupported(in);
	}

#undef detail

void ignore (Event *ev, Context *ctx) {
	UNUSED(ev);
	UNUSED(ctx);
}

