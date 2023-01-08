#include "row.h"

#include <stdlib.h>
#include <string.h>

#include "syntax.h"

int row_x_to_display_x(struct editor_row* row, int x)
{
	int display_x = 0;
	int j;

	for (j = 0; j < x; j++)	{
		if (row->chars[j] == '\t')
			display_x += (TAB_WIDTH - 1) - (display_x % TAB_WIDTH);
		display_x++;
	}

	return display_x;
}

int row_display_x_to_x(struct editor_row* row, int display_x)
{
	int current_display_x = 0;
	int result_x;

	for (result_x = 0; result_x < row->size; result_x++) {
		if (row->chars[result_x] == '\t')
			result_x += (TAB_WIDTH - 1) - (current_display_x % TAB_WIDTH);
		result_x++;

		if (current_display_x > display_x)
			return result_x;
	}
	return result_x;
}

void update_row(struct editor_state* editor, struct editor_row* row)
{
	int tabs = 0;
	int j;
	for (j = 0; j < row->size; j++)
		if (row->chars[j] == '\t') tabs++;

	free(row->render);
	row->render = malloc(row->size + tabs * (TAB_WIDTH - 1) + 1);

	int index = 0;
	for (j = 0; j < row->size; j++)	{
		if (row->chars[j] == '\t') {
			row->render[index++] = ' ';
			while (index % TAB_WIDTH != 0) row->render[index++] = ' ';
		} else {
			row->render[index++] = row->chars[j];
		}
	}
	
	row->render[index] = '\0';
	row->render_size = index;

	editor_update_syntax(editor, row);
}

void insert_row(struct editor_state* editor, int at, char* string, size_t length)
{
	if (at < 0 || at > editor->row_count)
		return;

	editor->rows = realloc(editor->rows, sizeof(struct editor_row) * (editor->row_count + 1));
	memmove(&editor->rows[at + 1], &editor->rows[at], sizeof(struct editor_row) * (editor->row_count - at));

	for (int j = at + 1; j <= editor->row_count; j++)
		editor->rows[at].index++;
		
	editor->rows[at].index = at;

	editor->rows[at].size = length;
	editor->rows[at].chars = malloc(length + 1);
	memcpy(editor->rows[at].chars, string, length);
	editor->rows[at].chars[length] = '\0';

	editor->rows[at].render_size = 0;
	editor->rows[at].render = NULL;
	editor->rows[at].highlight = NULL;
	editor->rows[at].highlight_open_comment = 0;
	update_row(editor, &editor->rows[at]);
	
	editor->row_count++;
	editor->dirty = 1;
}

void free_row(struct editor_row* row)
{
	free(row->render);
	free(row->chars);
	free(row->highlight);
}

void delete_row(struct editor_state* editor, int at)
{
	if (at < 0 || at >= editor->row_count)
		return;

	free_row(&editor->rows[at]);
	memmove(&editor->rows[at], &editor->rows[at + 1], sizeof(struct editor_row) * (editor->row_count - at - 1));

	for (int j = at; j < editor->row_count; j++)
		editor->rows[j].index--;

	editor->row_count--;
	editor->dirty = 1;
}

void row_insert_char(struct editor_state* editor, struct editor_row* row, int at, int c)
{
	if (at < 0 || at > row->size)
		at = row->size;

	row->chars = realloc(row->chars, row->size + 2);
	memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
	row->size++;
	row->chars[at] = c;

	update_row(editor, row);

	editor->dirty = 1;
}

void row_append_string(struct editor_state* editor, struct editor_row* row, char* string, size_t length)
{
	row->chars = realloc(row->chars, row->size + length + 1);
	memcpy(&row->chars[row->size], string, length);
	row->size += length;
	row->chars[row->size] = '\0';

	update_row(editor, row);
	editor->dirty = 1;
}

void row_delete_char(struct editor_state* editor, struct editor_row* row, int at)
{
	if (at < 0 || at >= row->size)
		return;

	memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
	row->size--;
	update_row(editor, row);
	editor->dirty = 1;
}
