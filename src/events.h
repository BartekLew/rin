#ifndef __H_EVENTS
#define __H_EVENTS

#include "common.h"
#include "calibration.h"

#include <linux/input.h>

typedef void (*Handler) (Context *ctx);

struct context {
	uint		completeness, pressure;
	Point		point, last;
	Handler		point_handler;

	Calibration	calibration;
};

/* Completeness flags */
#define CTX_X		0x01
#define CTX_Y		0x02
#define CTX_PRESSURE	0x04
#define CTX_UNIQ_PRESS	0x08
#define CTX_TOUCH	(CTX_X | CTX_Y | CTX_PRESSURE | CTX_UNIQ_PRESS)

typedef struct {
	Handler init, point, conclusion;
} Application;


bool event_app (const char *dev, Application app);

#endif
