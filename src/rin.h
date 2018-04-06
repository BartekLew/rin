#ifndef __H_RIN
#define __H_RIN

#include "common.h"
#include <linux/input.h>


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

#endif 