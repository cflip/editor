#pragma once

struct append_buffer {
	char* buffer;
	int length;
};

#define ABUF_INIT { NULL, 0 }

void ab_append(struct append_buffer* ab, const char* string, int length);
void ab_free(struct append_buffer* ab);
