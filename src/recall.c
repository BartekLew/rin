#include "interface.h"
#include "common.h"

#include <fblib.h>


int fb_main (Screen s, Strings args) {
	SerializedPoint p;

	while (fread (&p, sizeof(p), 1, stdin) == 1) {
		on_point (s, (Point){
			.x = p.x * s.width,
			.y = p.y * s.height
		});
	}

	return 0;
}
