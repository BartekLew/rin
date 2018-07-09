#ifndef __H_COMMON
#define __H_COMMON

/* Stuff that should be default. */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <fblib.h>

#define UNUSED(var) (void)(var)

#define _u (unsigned int)
#define _s (int)
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int_fast32_t sint;

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
			(unsigned int) (ev)->time.tv_sec, (unsigned int) (ev)->time.tv_usec, \
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
	printf ("(D) in %s @ %u\n", __FILE__, __LINE__); \
	exit(42); \
}

#define Warn(...) { \
	printf (__VA_ARGS__); \
	printf ("(W) in %s @ %u\n", __FILE__, __LINE__); \
}

#define loop(I,Len) \
	for (uint I = 0; I < Len; I++)

typedef struct input_event Event;

typedef struct {
	uint x, y;
} Point;

typedef struct {
	sint x, y;
} Mov;

typedef struct context Context;

#define Read "r"
#define Write "w"
#define optional_file(Name, Mode, Handle) \
	FILE *Handle = fopen (Name, Mode); \
	if (Handle != NULL)

#define read_one(Var, Stream) \
	fread (&Var, sizeof (Var), 1, Stream)

#define write_one(Var, Stream) \
	fwrite (&Var, sizeof (Var), 1, Stream)

#define write_one_w(Var, Stream, Name) \
	if (fwrite (&Var, sizeof (Var), 1, Stream) != 1) \
		Die ("Write %u bytes to %s failed @ %u.\n", \
			 _u sizeof(Var), Name, _u ftell(Stream) \
		);

#define read_exact(Arr, Count, Stream) \
	(fread (Arr, sizeof(*Arr), Count, Stream) == Count)

#define write_exact(Arr, Count, Stream) \
	(fwrite (Arr, sizeof(*Arr), Count, Stream) == Count)

#define write_exact_w(Arr, Count, Stream, Name) \
	if (fwrite (Arr, sizeof(*Arr), Count, Stream) != Count) \
		Die ("Write %u bytes to %s failed @ %u.\n", \
			 _u (sizeof(*Arr)*Count), Name, _u ftell(Stream) \
		);

#endif
