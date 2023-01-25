#include "textbuf.h"

#include <stdlib.h>
#include <string.h>

#include "error.h"

struct textbuf textbuf_init()
{
	struct textbuf result;
	result.buffer = NULL;
	result.length = 0;
	return result;
}

void textbuf_append(struct textbuf *textbuf, const char *str, int len)
{
	char *new = realloc(textbuf->buffer, textbuf->length + len);
	if (new == NULL) {
		fatal_error("Failed to reallocate textbuf!");
		return;
	}

	memcpy(&new[textbuf->length], str, len);
	textbuf->buffer = new;
	textbuf->length += len;
}

void textbuf_free(struct textbuf *textbuf)
{
	free(textbuf->buffer);
}
