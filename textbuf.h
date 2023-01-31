/*
 * textbuf.h: A simple dynamic text buffer.
 *
 * This file provides the `textbuf` struct, which is a dynamically allocated
 * buffer of text.
 */

#ifndef _BUFFER_H
#define _BUFFER_H

#include <stddef.h>

struct textbuf {
	char  *buffer;
	size_t length;
};

struct textbuf textbuf_init();
void textbuf_append(struct textbuf *textbuf, const char *str, int len);
void textbuf_delete(struct textbuf *textbuf);
void textbuf_clear(struct textbuf *textbuf);
void textbuf_free(struct textbuf *textbuf);

#endif
