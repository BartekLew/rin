#include "interface.h"

#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <ctype.h>

#define sub_or_zero(A,B) ((A > B) ? A - B : 0)
#define add_or_max(A,B,Max) ((A+B < Max) ? A+B : Max)

#define around_square(S, X, Y, R) \
	for (uint y = sub_or_zero (Y,R); y <= add_or_max (Y,R,S.height); y++)\
		for (uint x = sub_or_zero(X,R); x <= add_or_max (X,R,S.width); x++)

void blur_point (Screen s, Point p, uint r, Color c) {
	around_square (s, p.x, p.y, r) {
		float	dx = (float)x - (float)p.x,
			dy = (float)y - (float)p.y;
		float	ar = sqrtf(dx*dx + dy*dy);

		if (ar <= r) {
			dot_rgb (s, x, y, c.r, c.g, c.b);
		}
	}
}

Color brush_color = { .r = 25, .g = 142, .b = 100 };
unsigned int brush_size = 10;
FILE *record_file = NULL;
uint record_start = 0;

void on_point (Screen s, Point p) {
	if (record_file != NULL) {
		SerializedPoint pos = { .x = p.x, .y = p.y,
			.time = get_time() - record_start, .color = brush_color,
			.size = brush_size
		};
		fwrite (&pos, sizeof(pos), 1, record_file);
	}


	blur_point (s, p, brush_size, brush_color);
}

void close_record(void) {
	if (record_file != NULL) {
		fclose (record_file);
		record_file = NULL;
	}
}

uint get_time (void) {
	static struct timespec base = {0};

	struct timespec t;
	clock_gettime (CLOCK_REALTIME, &t);

	if (base.tv_sec == 0 && base.tv_nsec == 0) {
		base = t;
		return 0;
	} else {
		return (t.tv_sec - base.tv_sec) * 100
			+ (t.tv_nsec - base.tv_nsec) / 10000000;
	}
}

pthread_t console;
#define Cmd_max 80

typedef void (*Action) (Screen *s, char *arg, Color mask);
typedef struct {
	Color	mask;
	Action	action;
	char	token;
} CmdTok;

void color_act (Screen *s, char *arg, Color mask) {
	UNUSED(s);
	unsigned int val;

	if (sscanf (arg, "%x", &val) != 1)
		Warn ("wrong parameter: %s", arg)
	else {
		if (mask.r != 0x00)
			brush_color.r = val;
		if (mask.g != 0x00)
			brush_color.g = val;
		if (mask.b != 0x00)
			brush_color.b = val;
	}
}

#define File_max 20

void load_act (Screen *s, char *arg, Color mask) {
	UNUSED(mask);

	char file[File_max];
	sscanf (arg, "%s", file);
	FILE *i = fopen (file, "r");
	if (i == NULL)
		Warn ("Unable to open file: %s", file)
	else {
		if (fread (s->buffer, s->size, 1, i) != 1)
			Warn ("write error: %s", file);
		fclose(i);
	}
}

void diff_act (Screen *s, char *arg, Color mask) {
	UNUSED(mask);

	char file[File_max];
	sscanf (arg, "%s", file);
	FILE *i = fopen (file, "r");
	if (i == NULL)
		Warn ("Unable to open file: %s", file)
	else {
		char ibuf[s->size];
		Screen is = *s;
		is.buffer = ibuf;

		if (fread (ibuf, s->size, 1, i) != 1)
			Warn ("write error: %s", file);

		for (uint y = 0; y < is.height; y++){
			for (uint x = 0; x < is.width; x++) {
				Color ic = get_rgb (is, x, y); 
				Color c = get_rgb (*s, x, y);

				dot_rgb(*s, x, y, abs(ic.r-c.r), abs(ic.g-c.g), abs(ic.b-c.b));
			}
		}

		fclose(i);
	}
}

void store_act (Screen *s, char *arg, Color mask) {
	UNUSED(mask);

	char file[File_max];
	sscanf (arg, "%s", file);
	FILE *o = fopen (file, "w");
	if (o == NULL)
		Warn ("Unable to open file: %s", file)
	else {
		if (fwrite (s->buffer, s->size, 1, o) != 1)
			Warn ("write error: %s", file);
		fclose(o);
	}
}

void clear_act (Screen *s, char *arg, Color mask) {
	UNUSED(arg);
	UNUSED(mask);

	for (uint y = 0; y < s->height; y++) {
		for (uint x = 0; x < s->width; x++) {
			dot_rgb (*s, x, y,
				brush_color.r, brush_color.g, brush_color.b
			);
		}
	}
}

void size_act (Screen *s, char *arg, Color mask) {
	UNUSED(s);
	UNUSED(mask);

	unsigned int size;
	if (sscanf (arg, "%x", &size) != 1)
		Warn ("Expected size, got %s.", arg)
	else {
		brush_size = size;
	}
}

void record_act (Screen *s, char *arg, Color mask) {
	UNUSED(s);
	UNUSED(mask);
	
	if (record_file != NULL)
		close_record();

	if (isalpha(arg[0])) {
		char file[File_max];
		sscanf (arg, "%s", file);
		record_file = fopen (file, "w");
		record_start = get_time();
	}	
}

void replay_act (Screen *s, char *arg, Color mask) {
	UNUSED(mask);
	
	if (isalpha(arg[0])) {
		char file[File_max];
		sscanf (arg, "%s", file);
		FILE *record = fopen (file, "r");
		if (record == NULL)
			return;
		SerializedPoint p;
		uint start_time = get_time();

		Color c = brush_color;
		uint os = brush_size;

		while (fread (&p, sizeof(p), 1, record) == 1) {
			while (p.time + start_time > get_time());
			brush_size = p.size;
			brush_color = p.color;
			on_point (*s, (Point){
				.x = p.x,
				.y = p.y
			});
		}

		fclose (record);

		brush_color = c;
		brush_size = os;
	}	
}

#define Color_tok(KEY) .token = KEY, .action = &color_act

CmdTok command_tokens[] = {
	{Color_tok('r'), .mask = {.r = 0xff}},
	{Color_tok('g'), .mask = {.g = 0xff}},
	{Color_tok('b'), .mask = {.b = 0xff}},
	{Color_tok('x'), .mask = {.r = 0xff, .g = 0xff, .b = 0xff}},
	{.token = 's', .action = &store_act},
	{.token = 'l', .action = &load_act},
	{.token = 'd', .action = &diff_act},
	{.token = 'c', .action = &clear_act},
	{.token = 'p', .action = &size_act},
	{.token = '>', .action = &record_act},
	{.token = '<', .action = &replay_act}
};

void *prompt (char *buf, size_t buff_size) {
	printf ("%.2x;%.2x;%.2x>", brush_color.r, brush_color.g, brush_color.b);
	return fgets (buf, buff_size, stdin);
}

void *command_loop (void *ctx) {
	Screen *s = (Screen *)ctx;
	
	char cmd[Cmd_max];
	while (prompt (cmd, Cmd_max) != NULL) {
		for (uint i = 0; i < sizeof(command_tokens)/sizeof(CmdTok); i++) {
			CmdTok tok = command_tokens[i];
			if (tok.token == cmd[0]) {
				tok.action (s, cmd+1, tok.mask);
			}
		}
	}

	return NULL;
}

void listen_commands(Screen *s) {
	if (pthread_create (&console, NULL, &command_loop, s) != 0)
		Die ("Unable to spawn thread");
}

