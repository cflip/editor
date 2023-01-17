#ifndef _LINE_H
#define _LINE_H

#include <stddef.h>

#define TAB_WIDTH 4

typedef struct {
	int index;
	int size;
	char* chars;
	int render_size;
	char* render;
	unsigned char* highlight;
	int highlight_open_comment;
} line_t;

struct editor_state;

int row_x_to_display_x(line_t*, int x);
int row_display_x_to_x(line_t*, int display_x);

void editor_update_line(struct editor_state*, line_t*);
void editor_insert_line(struct editor_state*, int at, char *string, size_t length);
void editor_delete_line(struct editor_state*, int at);

void line_insert_char(struct editor_state*, line_t*, int at, int c);
void line_append_string(struct editor_state*, line_t*, char* string, size_t length);
void line_delete_char(struct editor_state*, line_t*, int at);

void free_line(line_t*);

#endif
