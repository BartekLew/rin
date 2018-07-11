// shape db tool

#include "shape_db.h"
#include "stat.h"

#include <stdio.h>

Stat len_stat = {0};

#define Max_len 0xf
Stat stat_coord[Max_len*2] = {0};

void print_shapes (SmallPoint *sh, uint len) {
	to_stat(&len_stat, len);
	loop (i, len) {
		if (i < Max_len) {
			to_stat(stat_coord+2*i, sh[i].x);
			to_stat(stat_coord+2*i+1, sh[i].y);
		}
		printf ("%u %u\t", sh[i].x, sh[i].y);
	}
	printf ("\n");
}

int main (int argc, char **argv) {
	if (argc == 2 && argv[1][1] == '\0') {
		load_shapes(Letter_db);
		iterate_letter(argv[1][0], print_shapes);
		print_stat(len_stat, "len");
		loop (i, len_stat.max) {
			char label[8];
			sprintf (label, "x%u", _u i);
			print_stat(stat_coord[2*i], label);
			sprintf (label, "y%u", _u i);
			print_stat(stat_coord[2*i+1], label);
		}
	}

	return 0;
}
