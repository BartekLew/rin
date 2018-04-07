#include "events.h"
#include "common.h"

void point (const Point *p) {
	printf ("cur @ %6u,%6u %6u\r", p->x, p->y, p->pressure);
}

int main (int argc, char **argv) {
	if (argc != 2)
		return 1;

	if (!event_loop (argv[1], &point))
		return 2;

	return 0;
}
