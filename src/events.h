#ifndef __H_EVENTS
#define __H_EVENTS

#include "common.h"

#include <linux/input.h>
#include <signal.h>
#include <fblib.h>

typedef void (*Handler) (Context *ctx);

typedef struct {
	Handler		init, touch, move, release, conclusion;
	void		(*sigint) (int);
} Application;

struct context {
	uint		completeness, pressure;
	bool		touching;
	Point		point, last;
	Application	app;
};

/* Completeness flags */
#define CTX_X		0x01
#define CTX_Y		0x02
#define CTX_PRESSURE	0x04
#define CTX_UNIQ_PRESS	0x08
#define CTX_TOUCH	(CTX_X | CTX_Y | CTX_PRESSURE | CTX_UNIQ_PRESS)


bool event_app (const char *dev, Application app);

#endif
