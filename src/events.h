#ifndef __H_EVENTS
#define __H_EVENTS

#include "common.h"

#include <linux/input.h>

typedef struct context Context;
typedef void (*Handler) (Context *ctx);

struct context {
	uint		completeness;
	uint		x, y, pressure;
	Handler		point_handler;
};

/* Completeness flags */
#define CTX_X		0x01
#define CTX_Y		0x02
#define CTX_PRESSURE	0x04
#define CTX_UNIQ_PRESS	0x08
#define CTX_TOUCH	(CTX_X | CTX_Y | CTX_PRESSURE | CTX_UNIQ_PRESS)

bool event_loop (const char *dev, Handler point_handler);

#endif
