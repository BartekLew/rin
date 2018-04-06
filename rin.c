#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <linux/input.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define UNUSED(var) (void)(var)

#ifdef DEBUG

#define Unsupported(ev) \
	fprintf( stderr, "\nt=%lu.%06lu %x/%x UNSUPPORTED @ %s:%u\n", \
		(uint) (ev)->time.tv_sec, (uint) (ev)->time.tv_usec, \
		(ev)->type, (ev)->code, __FILE__, __LINE__ \
	);

#define Protocol_Assumption(desc, test) \
	if (!(test)) \
		fprintf( stderr, "\nFAILED %s @ %s: %u.\n", \
			desc,__FILE__, __LINE__ \
		);

#else /* NOT DEBUG: */

#define Unsupported(ev)
#define Protocol_Assumption(desc, test)

#endif


typedef uint_fast32_t uint;
typedef struct input_event Event;

typedef struct {
	uint    completeness;
	uint	x, y, pressure, id;
} Context;

/* Completeness flags */
#define CTX_X		0x01
#define CTX_Y		0x02
#define CTX_PRESSURE	0x04
#define CTX_UNIQ_PRESS	0x08
#define CTX_TOUCH	(CTX_X | CTX_Y | CTX_PRESSURE | CTX_UNIQ_PRESS)

typedef void (*Handler) (Event *in, Context *ctx);

void sync (Event *in, Context *ctx) {

	if (ctx->completeness & CTX_TOUCH) {
		/*
			Not all fields are mandatory to happen, if not
			assume value didn't change.
		*/

		printf( "cur @ %6lu,%6lu %6lu %6lu\n",
			ctx->x, ctx->y, ctx->pressure, ctx->id
		);
	}

	ctx->completeness = 0;

}

void set_point (Event *in, Context *ctx) {
	if (in->code == ABS_X) {
		Protocol_Assumption ("ABS.X happens once",
			(ctx->completeness & CTX_X) == 0
		);
		ctx->completeness |= CTX_X;

		ctx->x = in->value;
	} else if (in->code == ABS_Y) {
		Protocol_Assumption ("ABS.Y happens once",
			(ctx->completeness & CTX_Y) == 0
		);
		ctx->completeness |= CTX_Y;

		ctx->y = in->value;
	} else if (in->code == ABS_PRESSURE) {
		Protocol_Assumption ("ABS.PRESSURE happens once",
			(ctx->completeness & CTX_PRESSURE) == 0
		);
		ctx->completeness |= CTX_PRESSURE;

		ctx->pressure = in->value;
	} else if (in->code == ABS_MT_TRACKING_ID) {
		Protocol_Assumption ("ABS.ID happends once",
			(ctx->completeness & CTX_UNIQ_PRESS) == 0
		);
		ctx->completeness |= CTX_UNIQ_PRESS;

		ctx->id = in->value;
	} else	Unsupported(in);
}

void ignore (Event *ev, Context *ctx) {
	UNUSED(ev);
	UNUSED(ctx);
}

Handler handlers[] = {
	[EV_SYN] = sync,
	[EV_ABS] = set_point,
	[EV_KEY] = ignore
};
#define Handlers_Cnt sizeof(handlers) / sizeof(Handler)

int main (int argc, char **argv) {
	if (argc != 2)
		return 1;
	int fd = open (argv[1], O_RDONLY);
	if (fd < 0)
		return 2;

	
	Event	e;
	Context	ctx;
	while (read (fd, &e, sizeof(e)) == sizeof(e)) {
		if (e.type < Handlers_Cnt && handlers[e.type] != NULL)
			handlers[e.type](&e, &ctx);
		else Unsupported(&e);
	}

	close(fd);
	return 0;
}
