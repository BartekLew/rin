#ifndef __H_CALIBRATION
#define __H_CALIBRATION

#include "events.h"

#define Calibration_file ".rin.dat"

void before_calibration (Context *ctx);
void calibration_point (Context *ctx);
void finish_calibration (Context *ctx);

#endif
