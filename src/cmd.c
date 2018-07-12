#include "cmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

static void cmd_drop(void);
static void cmd_list(void);
static void cmd_put_meaning(void);
static void cmd_quit(void);

Command commands[] = {
	{.tok = "ls", &cmd_list},
	{.tok = "=", &cmd_put_meaning},
	{.tok = "drop", &cmd_drop},
	{.tok = "q", &cmd_quit}
};

static char *prompt(void) {
	static char cmd[33];

	printf ("> ");
	if (scanf ("%32s", cmd) == 1) {
		return cmd;	
	}

	return NULL;
}

static void *command_loop (void *ctx) {
	UNUSED(ctx);
	
	char *tok;
	while ((tok = prompt()) != NULL) {
		bool found = false;
		in (i, commands) {
			if (strcmp (commands[i].tok, tok) == 0) {
				commands[i].action();
				found = true;
				break;
			}
		}

		if (!found)
			Warn ("Unknown command: %s\n", tok);
	}

	return NULL;
}

pthread_t console;
void listen_commands(void) {
	if (pthread_create (&console, NULL, &command_loop, NULL) != 0)
		Die ("Unable to spawn thread");
}

static void cmd_quit(void) {
	cmd_drop();
	store_shapes(Letter_db);
	exit(0);
}

#define Stack_size 0x20

Shape *shape_stack[Stack_size];
uint   shstack_ptr = 0;

void push_shape (Shape *sh) {
	if (shstack_ptr == Stack_size) {
		Warn ("Shape stack full");
	} else
		shape_stack[shstack_ptr++] = sh;
}

static void cmd_list(void) {
	loop (i, shstack_ptr) {
		if (shape_stack[i]->meaning != 0)
			printf ("%c", shape_stack[i]->meaning);
		else
			printf ("?");
	}
	printf ("\n");
}

static void cmd_drop(void) {
	if (shstack_ptr > 0)
		drop_changes();

	shstack_ptr = 0;
}

static void cmd_put_meaning(void) {
	char meaning[33];
	scanf ("%s", meaning);
	if (strlen(meaning) != shstack_ptr)
		Warn ("stack size should match put meaning")
	else {
		loop (i, shstack_ptr) {
			shape_stack[i]->meaning = meaning[i];
		}
		apply_changes();
		shstack_ptr = 0;
	}
}
