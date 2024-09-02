#include "events.h"
#include "common.h"

#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>


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

#define SPACE_WEIGHT 50.0

/* Having shape in pointbuff with points_in_buff storing
   number of points in shape, i use normalize_shape()
   to transform coordinates to relative to minimal rectancle
   enclosing the shape with values 0-SPACE_WEIGHT.
   SPACE_WEIGHT impacts how revelant time component is going
   to be.*/
void normalize_shape (void) {
    uint w = max.x - min.x + 1;
    uint h = max.y - min.y + 1;

    for (uint i = 0; i < points_in_buff; i++) {
        uint normx = (double) (pointbuff[i].x - min.x) * SPACE_WEIGHT / (double) w;
        uint normy = (double) (pointbuff[i].y - min.y) * SPACE_WEIGHT / (double) h;

        pointbuff[i] = (Point) { .x = normx, .y = normy };
    }
}

/* When recognizing shapes, I'm going to find matches by 3 dimensional
   distance between points in pattern and tested shape. The pattern with
   smallest average distance is getting matched. Similarly, when calculating
   pattern, points with closest distance are matched. */
double point_distance(uint t1, Point p1, uint t2, Point p2) {
    double dt = _d(t2 - t1);
    double dx = _d(p2.x - p1.x);
    double dy = _d(p2.y - p1.y);

    return cbrt(dt*dt + dx*dx + dy*dy);
}

/* Because of matching point procedure in creating patterns, we need additional
   component of number of hits, so that average can be computed. I add also time
   component because it's goint to averaged as well. */
typedef struct pattern_point {
    uint time, x, y, hits;
} PatternPoint;

/* pattern_buff and points_in_patt are analogical to pointbuff and points_in_patt
   but for computing pattern */
PatternPoint pattern_buff[BUFF_SIZE];
uint points_in_patt = 0;

/* Initializing pattern is just copying first shape into pattern_buff */
void init_pattern(void) {
    points_in_patt = points_in_buff;
    for(uint i = 0; i < points_in_buff; i++) {
        pattern_buff[i] = (PatternPoint) {
            .time = i,
            .x = pointbuff[i].x, 
            .y = pointbuff[i].y, 
            .hits = 1
        };
    }
}

/* In next iterations, we have to look for matches… */
void next_to_pattern(void) {
    for(uint i = 0; i < points_in_buff; i++) {
        uint closest_id = 0;
        double best_distance = point_distance(i, _p(pointbuff[i]),
                pattern_buff[0].time, _p(pattern_buff[0]));
        for(uint j = 1; j < points_in_patt; j++) {
            double distance = point_distance(i, _p(pointbuff[i]),
                    pattern_buff[i].time, _p(pattern_buff[i]));
            if(distance < best_distance) {
                closest_id = j;
                best_distance = distance;
            }
        }

        pattern_buff[closest_id].x += pattern_buff[i].x;
        pattern_buff[closest_id].y += pattern_buff[i].y;
        pattern_buff[closest_id].time += i;
        pattern_buff[closest_id].hits++;
    }
}

/* Before pattern can be used, averages must be computed */
void finalize_pattern(void) {
    for(uint i = 0; i < points_in_patt; i++) {
        pattern_buff[i].x /= pattern_buff[i].hits;
        pattern_buff[i].y /= pattern_buff[i].hits;
        pattern_buff[i].time /= pattern_buff[i].hits;
    }
}

/* Having essential procedures for pattern making, we can
   define pattern->utf-8 character pairs. */
typedef struct pattern_mapping {
    PatternPoint points[BUFF_SIZE];
    uint points_count;

    char text[8];
    uint text_len;
} PatternMapping;

/* As Map of patterns is going to mmap()ed , it has to be
   a structure */
typedef struct pattern_map {
    uint map_len;
    PatternMapping data[1];
} PatternMapHeader;

int pattmap_fd = 0;
PatternMapHeader *patt_map = NULL;
#define PATTMAP_FILE ".rin.patt"

/* Then come map control functions:
      push_pattern(text, text_len) when adding pattern mode with
                                   given text to be attached to pattern.
      load_patterns() when regular mode, opening patterns for reading.
      close_patterns() to close patterns file */
void push_pattern(char *text, uint text_len) {
    uint maplen;
    if(patt_map == NULL) {
        pattmap_fd = open(PATTMAP_FILE, O_RDONLY);
        if(pattmap_fd > 0) {
            if(read(pattmap_fd, &maplen, sizeof(maplen)) != sizeof(maplen)) {
                fprintf(stderr, "Error reading pattern file: %s\n", PATTMAP_FILE);
                maplen = 0;
            }
            close(pattmap_fd);
        } else {
            maplen = 0;
        }
    } else {
        maplen = patt_map->map_len;
        close(pattmap_fd);
    }

    maplen++;
    pattmap_fd = open(PATTMAP_FILE, O_RDWR|O_CREAT, (mode_t) 0600);
    if(pattmap_fd > 0) {
        uint mapsize = sizeof(PatternMapHeader) + (maplen-1)*sizeof(PatternMapping);

        ftruncate(pattmap_fd, mapsize);
        patt_map = mmap(0, mapsize, PROT_READ|PROT_WRITE, MAP_SHARED, pattmap_fd, 0);
        if(patt_map == MAP_FAILED) {
            fprintf(stderr, "error mmap() on %s\n", PATTMAP_FILE);
            exit(1);
        }

        patt_map->map_len = maplen;
    } else {
        fprintf(stderr, "Can't open for writing: %s\n", PATTMAP_FILE);
        exit(1);
    }

    patt_map->data[maplen-1].text_len = text_len;
    memcpy(patt_map->data[maplen-1].text, text, text_len);
    memcpy(patt_map->data[maplen-1].points, pattern_buff, points_in_patt*sizeof(PatternPoint));
    patt_map->data[maplen-1].points_count = points_in_patt;
    patt_map->map_len = maplen;
}

void load_patterns(void) {
    if(patt_map == NULL) {
        pattmap_fd = open(PATTMAP_FILE, O_RDONLY);
        if(pattmap_fd <= 0) {
            fprintf(stderr, "Can't open for reading: %s (patterns file) \n", PATTMAP_FILE);
            exit(1);
        }

        uint maplen;
        if(read(pattmap_fd, &maplen, sizeof(maplen)) != sizeof(maplen)) {
            fprintf(stderr, "error reading pattern file: %s\n", PATTMAP_FILE);
            exit(1);
        }

        patt_map = mmap(0, sizeof(PatternMapHeader) + maplen*sizeof(PatternMapping),
                   PROT_READ, MAP_SHARED, pattmap_fd, 0);
    }
}

void close_patterns(void) {
    if(patt_map != NULL) {
        munmap(patt_map, patt_map->map_len);
        close(pattmap_fd);
    }
}

/* This function gets a point and fills pointbuff
   buffer. Return value indicates if touch has been
   released. If it returns false, it means that shape
   is in progress. Otherwise it means that
   records are placed and normalized in the buffer. 
   After handling shape, points_in_buff should be set
   to 0, so that new shape is started. */
uint calibrated_point (Context *ctx) {
    if(ctx->pressure == 0) {
        if(points_in_buff < 10) {
            points_in_buff = 0;
            return 0;
        }

        normalize_shape();

        min = MAXPOINT;
        max = MINPOINT;

        uint ans = points_in_buff;
        
        ctx->last = ctx->point;

        return ans;
    } else {
        if (points_in_buff > 0
            && ctx->last.x - ctx->point.x < Calib(ctx)->threshold.x
            && ctx->last.y - ctx->point.y < Calib(ctx)->threshold.y)

            return 0;

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

        return 0;
    }
}

/* First we have to teach program to recognize patterns.
   each pattern is made of SHAPES_PER_PATTERN shapes.
   learning_point(ctx) is the point handler for that
   mode of running. */
#define SHAPES_PER_PATTERN 10
uint collected_shapes = 0;

void learning_point (Context *ctx) {
    if (calibrated_point(ctx)) {
        if (collected_shapes == 0) {
            init_pattern();
        } else {
            next_to_pattern();
        }

        points_in_buff = 0;

        if(++collected_shapes == SHAPES_PER_PATTERN)
            ctx->point_handler = NULL;

        fprintf(stderr, "Progress: %lu/%d\r", collected_shapes, SHAPES_PER_PATTERN);
    }
}

/* Otherwise we check which pattern fits best to the shape
   drawn by user. */

/* TODO: following procedure tries to match points included in pattern to
   points recorded. For now, I can't really say how does it impact the process
   except that some shapes are unequally more likely to be matched than others.

   So the whole process should be redesigned to make it more accurate. */
double score_pattern(uint pattno) {
    double score = 0.0;
    PatternMapping *patt = &(patt_map->data[pattno]);
    for(uint pointno = 0; pointno < patt->points_count; pointno++) {
        double best_distance = point_distance(0, _p(pointbuff[0]),
                                              patt->points[pointno].time, _p(patt->points[pointno]));
        for(uint j = 1; j < points_in_buff; j++) {
            double distance = point_distance(j, _p(pointbuff[j]),
                                             patt->points[pointno].time, _p(patt->points[pointno]));
            if(distance < best_distance) {
                best_distance = distance;
            }
        }

        score += best_distance;
    }

    score /= patt->points_count;
    return score;
}

uint choose_pattern(void) {
    uint best_patt = 0;
    double best_score = score_pattern(0);
    for(uint pattno = 0; pattno < patt_map->map_len; pattno++) {
        double score = score_pattern(pattno);
        if(score < best_score) {
            best_patt = pattno;
            best_score = score;
        }
    }

    return best_patt;
}

void writing_point (Context *ctx) {
    if(calibrated_point(ctx)) {
        uint patt = choose_pattern();
        printf("%.*s", (int) patt_map->data[patt].text_len, patt_map->data[patt].text);
        points_in_buff = 0;
    }
}

char *saving_text;
uint saving_text_len;
int main (int argc, char **argv) {
    /* There are two modes 
            * rin [dev] to write using patterns
            * rin [dev] [text] to store pattern for text */
	if (argc == 2) {
        load_patterns();

        if(patt_map->map_len < 2) {
            fprintf(stderr, "Can't work with less than 2 patterns.");
            return 2;
        }

        if (!event_app (argv[1], (Application) {
            .point		= &writing_point
        })) {
            return 1;
        }
    } else if (argc == 3) {
        saving_text = argv[2];
        saving_text_len = strlen(argv[2]);

        fprintf(stderr, "Learning shape for '%.*s':\nProgress: 0/%d\r",
                (int) saving_text_len, saving_text, SHAPES_PER_PATTERN);

        if (!event_app (argv[1], (Application) {
            .point		= &learning_point
        })) {
            return 1;
        }

        push_pattern(argv[2], strlen(argv[2]));
        fprintf(stderr, "Pattern %s added successfully\n", argv[2]);
    } else {
        fprintf(stderr, "usage: %s [dev]        — read shapes from dev output to stdout\n"
                        "       %s [dev] [text] — capture shape pattern from [dev] for given [text]\n",
                        argv[0], argv[0]);
		return 2;
    }

	return 0;
}
