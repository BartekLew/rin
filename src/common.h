#ifndef __H_COMMON
#define __H_COMMON

/* Stuff that should be default. */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


#define UNUSED(var) (void)(var)

#ifdef TESTIFY

	#define DEBUG
	
	#define Testify(...) \
		printf (__VA_ARGS__)
	
	#else
	
	#define Testify(...)

#endif /* TESTIFY */


#ifdef DEBUG

	#define Unsupported(ev) \
		fprintf( stderr, "t=%u.%06u %x/%x UNSUPPORTED @ %s:%u\n", \
			(uint) (ev)->time.tv_sec, (uint) (ev)->time.tv_usec, \
			(ev)->type, (ev)->code, __FILE__, __LINE__ \
		);
	
	#define Protocol_Assumption(desc, test) \
		if (!(test)) \
			fprintf( stderr, "FAILED %s @ %s: %u.\n", \
				desc,__FILE__, __LINE__ \
			);

#else /* NOT DEBUG: */

	#define Unsupported(ev)
	#define Protocol_Assumption(desc, test)

#endif

#define Die(...) { \
	printf (__VA_ARGS__); \
	printf ("in %s @ %u\n", __FILE__, __LINE__); \
	exit(42); \
}

typedef struct input_event Event;

#endif
