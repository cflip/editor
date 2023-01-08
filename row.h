#pragma once

#include <ctype.h>

#include "editor.h"

#define TAB_WIDTH 4

struct editor_row {
	int index;
	int size;
	char* chars;
	int render_size;
	char* render;
	unsigned char* highlight;
	int highlight_open_comment;
};

int row_x_to_display_x(struct editor_row* row, int x);
int row_display_x_to_x(struct editor_row* row, int display_x);
void update_row(struct editor_state* editor, struct editor_row* row);
void insert_row(struct editor_state* editor, int at, char* string, size_t length);
void free_row(struct editor_row* row);
void delete_row(struct editor_state* editor, int at);
void row_insert_char(struct editor_state* editor, struct editor_row* row, int at, int c);
void row_append_string(struct editor_state* editor, struct editor_row* row, char* string, size_t length);
void row_delete_char(struct editor_state* editor, struct editor_row* row, int at);
