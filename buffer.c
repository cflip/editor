#include "buffer.h"

#include <stdlib.h>
#include <string.h>

void ab_append(struct append_buffer* ab, const char* string, int length)
{
	char* new = realloc(ab->buffer, ab->length + length);

	if (new == NULL)
		return;

	memcpy(&new[ab->length], string, length);
	ab->buffer = new;
	ab->length += length;
}

void ab_free(struct append_buffer* ab)
{
	free(ab->buffer);
}
