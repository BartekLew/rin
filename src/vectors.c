#include "events.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	s32 x, y;
} Vector;

Vector total_vector;

#define Max_points 30

typedef struct {
	Vector	points[Max_points];
	size_t	points_cnt;
} Shape;
Shape current;

#define Letters_file ".letters.dat"
#define letters_cnt 'z' - 'a' + 1
Shape	letters[letters_cnt];
uint	current_letter = 0;

void add_point (Vector v) {
	if (abs(v.x) < 15 || abs(v.y) < 15)
		return;

	if (++current.points_cnt >= Max_points)
		Die ("too many points!");

	current.points[current.points_cnt] = (Vector) {
		.x = current.points[current.points_cnt-1].x + v.x,
		.y = current.points[current.points_cnt-1].y + v.y
	};
}

void print_vector(Vector v) {
	if (abs(v.x) > 15 || abs(v.y) > 15)
		printf ("%d %d\n", v.x, v.y); 
}

/* returns direction value so that it takes values <-20;20>,
   0 -> horizontal, +/-20 -> vertical, +/-10 -> 45deg
   sign respective to sign of tan function. */
s32 direction (Vector v) {
	if (v.x == 0 && v.y == 0) return 0;
	if (v.x < 0) {
		v.x = -v.x;
		v.y = -v.y;
	}

	if (v.y > v.x * 10) return 20;
	if (v.y < v.x * -10) return -20;
	s32 tg = 10 * v.y / v.x;
	if (abs(tg) < 10)
		return tg;
	else if (tg > 0)
		return 10 + tg/10;
	else
		return -10 + tg/10;
}

bool same_direction (Vector a, Vector b) {
	if (a.x * b.x < 0) {
		s32 dira = direction(a);
		if (dira < 0) dira += 40;
		s32 dirb = direction(b);
		if (dirb < 0) dira += 40;

		Testify ("dira=%d dirb=%d\n", dira, dirb);

		return abs (dira - dirb) < 10;
	} else {
		Testify ("da=%d db=%d\n", direction(a), direction(b));
		return abs (direction(a)-direction(b)) < 10;
	}
}

static void init (Context *ctx);
static void first_move (Context *ctx);
static void next_move (Context *ctx);
static void store_letter (Context *ctx);
static void recognize_letter (Context *ctx);

static void init (Context *ctx) {
	if (letters_cnt > current_letter)
		printf ("draw letter \"a\"...\n");
	else
		printf ("draw letter to recognize...\n");
}

static void first_move (Context *ctx) {
	total_vector = (Vector){
		.x = ctx->point.x - ctx->last.x,
		.y = ctx->point.y - ctx->last.y
	};
	current.points_cnt = 0;

	ctx->app.move = &next_move;
}

static void next_move (Context *ctx) {
	Vector v = {
		.x = ctx->point.x - ctx->last.x,
		.y = ctx->point.y - ctx->last.y
	};

	if (!same_direction(v, total_vector)) {
		add_point(total_vector);
		total_vector = v;
	} else {
		total_vector.x += v.x;
		total_vector.y += v.y;
	}
}

static Vector translate_point (uint id) {
	s32 new_x = 0, new_y = 0;
	for (size_t j = 0; j <= current.points_cnt; j++) {
		if (id!=j) {
			if (current.points[id].x - current.points[j].x > 20) new_x++;
			if (current.points[id].y - current.points[j].y > 20) new_y++;
		}
	}

	return (Vector) { .x = new_x, .y = new_y };
}

static void store_letter (Context *ctx) {
	add_point(total_vector);
	letters[current_letter].points_cnt = current.points_cnt;
	
	for (size_t i = 0; i <= current.points_cnt; i++) {
		letters[current_letter].points[i] = translate_point(i);
	}

	current_letter++;

	if (current_letter == letters_cnt) {
		bool success = false;
		optional_file (Letters_file, Write, lf) {
			if (write_exact (letters, letters_cnt, lf))
				success = true;
			fclose(lf);
		}
		if (!success)
			Warn ("Unable to write %u bytes to %s\n",
				_u sizeof(Shape)*letters_cnt, Letters_file
			);
		
		printf ("draw letter to recognize...\n");
		ctx->app.release = &recognize_letter;
	} else
		printf ("draw letter \"%c\"...\n", (int)('a' + current_letter));

	ctx->app.move = &first_move;
}

static void recognize_letter (Context *ctx) {
	add_point(total_vector);
	uint grades[letters_cnt] = {0};
	
	for (size_t i = 0; i <= current.points_cnt; i++) {
		Vector new = translate_point(i);

		for (uint li = 0; li < letters_cnt; li++)
			if (letters[li].points_cnt > i)
				grades[li] += abs(letters[li].points[i].x - new.x)
					+ abs(letters[li].points[i].y - new.y);
	}
	
	uint guess = 0;
	for (uint i = 0; i < letters_cnt; i++) {
		grades[i] *= abs(letters[i].points_cnt - current.points_cnt) +1;
		if (grades[i] < grades[guess]) guess = i;

		printf ("%c: %u, points %u/%u\n", (int)('a'+i), _u grades[i],
				_u letters[i].points_cnt, _u current.points_cnt);
	}
	printf ("my guess is \"%c\".\n", (int)('a'+guess));
	printf ("draw letter to recognize...\n");

	ctx->app.move = &first_move;
}

int main (int args_count, char **args) {
	if (args_count != 2)
		return 1;

	Handler release = &store_letter;
	optional_file (Letters_file, Read, lf) {
		if (read_exact (letters, letters_cnt, lf)) {
			current_letter = letters_cnt;
			release = &recognize_letter;
		}
		fclose(lf);
	}

	if (!event_app (args[1], (Application) {
		.init = &init,
		.move = &first_move,
		.release = release
	}))
		return 2;

	return 0;
}
