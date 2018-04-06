#include "rin.h"
#include "events.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

Handler handlers[] = {
	[EV_SYN] = sync,
	[EV_ABS] = set_point,
	[EV_KEY] = ignore
};
#define Handlers_Cnt sizeof(handlers) / sizeof(Handler)


int main (int argc, char **argv) {
	if (argc != 2)
		return 1;
	int fd = open (argv[1], O_RDONLY);
	if (fd < 0)
		return 2;

	
	Event	e;
	Context	ctx;
	while (read (fd, &e, sizeof(e)) == sizeof(e)) {
		if (e.type < Handlers_Cnt && handlers[e.type] != NULL)
			handlers[e.type](&e, &ctx);
		else Unsupported(&e);
	}

	close(fd);
	return 0;
}
