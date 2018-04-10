#ifndef __H_CALIBRATION
#define __H_CALIBRATION

#include "common.h"
#include "events.h"

#define Calibration_file ".rin.dat"

typedef struct {
	Point		min, max, threshold;
	int		up_down, left_right;	
} CalibrationData;

typedef struct {
	CalibrationData	*data;
	int		fd;
} Calibration;

Calibration get_calibration(int devfd);

#define Calib(Ctx) ((Ctx)->calibration.data)

void before_calibration (Context *ctx);
void calibration_point (Context *ctx);
void finish_calibration (Context *ctx);

#endif
