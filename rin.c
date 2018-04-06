#include <stdio.h>
#include <stdint.h>

#include <linux/input.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef uint_fast32_t uint;

int main (int argc, char **argv) {
	if (argc != 2)
		return 1;
	int fd = open (argv[1], O_RDONLY);
	if (fd < 0)
		return 2;

	
	struct input_event e;
	while (read (fd, &e, sizeof(e)) == sizeof(e)) {
		printf( "t=%lu.%06lu %x\n", (uint) e.time.tv_sec,
			(uint) e.time.tv_usec, e.type
		);
	}

	return 0;
}
