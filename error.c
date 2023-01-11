#include "error.h"

#include <stdio.h>
#include <stdlib.h>

void fatal_error(const char *msg)
{
	perror(msg);
	exit(-1);
}

void warning(const char *msg)
{
	fprintf(stderr, "WARNING: %s\n", msg);
}
