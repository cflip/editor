#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void fatal_error(const char *msgfmt, ...)
{
	va_list args;
	va_start(args, msgfmt);
	vfprintf(stderr, msgfmt, args);
	va_end(args);
	exit(-1);
}

void warning(const char *msg)
{
	fprintf(stderr, "WARNING: %s\n", msg);
}
