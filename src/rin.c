#include "events.h"
#include "common.h"

#include <time.h>
#include <math.h>

/* I want to identify shapes. So I need some buffer
   to store consecutive points before than can be
   interpreted */

#define BUFF_SIZE 1024
Point pointbuff[BUFF_SIZE];
uint points_in_buff = 0;

/* I want shapes size independent so I need to know
   boundaries of the shape. */
#define MINPOINT (Point) { .x = 0, .y = 0 }
#define MAXPOINT (Point) { .x = UINT_MAX, .y = UINT_MAX }
Point min = MAXPOINT;
Point max = MINPOINT;

/* For each buffer, I'm going to provide some measures */
typedef struct shape {
    double aspect_ratio; /* aspect ratio */

    /* Average coordinates, x/y and circular */
    double avg_x, avg_y, avg_angle, avg_distance; 
} Shape;

void calculate_shape (Shape *out) {
    double w = (double) (max.x - min.x);
    double h = (double) (max.y - min.y);

    if(abs(w) < 0.01 || abs(h) < 0.01)
        return;

    out->aspect_ratio = w/h;

    out->avg_x = out->avg_y = out->avg_angle = out->avg_distance = 0.0;

    for (uint i = 0; i < points_in_buff; i++) {
        double normx = (double) (pointbuff[i].x - min.x) / w;
        double normy = (double) (pointbuff[i].y - min.y) / h;

        out->avg_x += normx;
        out->avg_y += normy;

        double dist = sqrt (normx*normx + normy*normy);
        out->avg_distance += dist;

        if(abs(dist) > 0.01)
            out->avg_angle += normx / dist;
    }

    out->avg_x /= (double)points_in_buff;
    out->avg_y /= (double)points_in_buff;
    out->avg_distance /= (double)points_in_buff;
    out->avg_angle /= (double)points_in_buff;
}

void calibrated_point (Context *ctx) {
    if(ctx->pressure == 0) {
        if(points_in_buff == 0)
            return;

        Shape sh;
        calculate_shape(&sh);

        printf ("%lu %lf %lf %lf %lf %lf\n",
                points_in_buff, sh.aspect_ratio, sh.avg_x, sh.avg_y, sh.avg_angle, sh.avg_distance);

        min = MAXPOINT;
        max = MINPOINT;
        points_in_buff = 0;
    } else {
        if (ctx->last.x - ctx->point.x < Calib(ctx)->threshold.x
            && ctx->last.y - ctx->point.y < Calib(ctx)->threshold.y)

            return;

        if (ctx->pressure > 0 && points_in_buff < BUFF_SIZE) {
            uint x = ctx->point.x;
            uint y = ctx->point.y;
            pointbuff[points_in_buff++] = (Point) { .x = x, .y = y };

            min = (Point) { .x = (min.x > x)? x : min.x,
                            .y = (min.y > y)? y : min.y };
    
            max = (Point) { .x = (max.x < x)? x : max.x,
                            .y = (max.y < y)? y : max.y };
        } else {
            fprintf(stderr, "warning: Buffer overflow, stroke closed");
        }

        ctx->last = ctx->point;
    }
}

int main (int argc, char **argv) {
	if (argc != 2)
		return 1;

	if (!event_app (argv[1], (Application) {
		.point		= &calibrated_point
	}))
		return 2;

	return 0;
}
