#include "line.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "syntax.h"

int row_x_to_display_x(line_t *line, int x)
{
	int display_x = 0;
	int j;

	for (j = 0; j < x; j++)	{
		if (line->chars[j] == '\t')
			display_x += (TAB_WIDTH - 1) - (display_x % TAB_WIDTH);
		display_x++;
	}

	return display_x;
}

int row_display_x_to_x(line_t *line, int display_x)
{
	int current_display_x = 0;
	int result_x;

	for (result_x = 0; result_x < line->size; result_x++) {
		if (line->chars[result_x] == '\t')
			result_x += (TAB_WIDTH - 1) - (current_display_x % TAB_WIDTH);
		result_x++;

		if (current_display_x > display_x)
			return result_x;
	}
	return result_x;
}

void editor_update_line(struct editor_state *editor, line_t *line)
{
	int tabs = 0;
	int j;
	for (j = 0; j < line->size; j++)
		if (line->chars[j] == '\t') tabs++;

	free(line->render);
	line->render = malloc(line->size + tabs * (TAB_WIDTH - 1) + 1);

	int index = 0;
	for (j = 0; j < line->size; j++)	{
		if (line->chars[j] == '\t') {
			line->render[index++] = ' ';
			while (index % TAB_WIDTH != 0) line->render[index++] = ' ';
		} else {
			line->render[index++] = line->chars[j];
		}
	}
	
	line->render[index] = '\0';
	line->render_size = index;

	editor_update_syntax(editor, line);
}

void editor_insert_line(struct editor_state *editor, int at, char* string, size_t length)
{
	if (at < 0 || at > editor->num_lines)
		return;

	editor->lines = realloc(editor->lines, sizeof(line_t) * (editor->num_lines + 1));
	memmove(&editor->lines[at + 1], &editor->lines[at], sizeof(line_t) * (editor->num_lines - at));

	for (int j = at + 1; j <= editor->num_lines; j++)
		editor->lines[at].index++;
		
	editor->lines[at].index = at;

	editor->lines[at].size = length;
	editor->lines[at].chars = malloc(length + 1);
	memcpy(editor->lines[at].chars, string, length);
	editor->lines[at].chars[length] = '\0';

	editor->lines[at].render_size = 0;
	editor->lines[at].render = NULL;
	editor->lines[at].highlight = NULL;
	editor->lines[at].highlight_open_comment = 0;
	editor_update_line(editor, &editor->lines[at]);
	
	editor->num_lines++;
	editor->dirty = 1;
}

void free_line(line_t *line)
{
	free(line->render);
	free(line->chars);
	free(line->highlight);
}

void editor_delete_line(struct editor_state *editor, int at)
{
	if (at < 0 || at >= editor->num_lines)
		return;

	free_line(&editor->lines[at]);
	memmove(&editor->lines[at], &editor->lines[at + 1], sizeof(line_t) * (editor->num_lines - at - 1));

	for (int j = at; j < editor->num_lines; j++)
		editor->lines[j].index--;

	editor->num_lines--;
	editor->dirty = 1;
}

void line_insert_char(struct editor_state *editor, line_t *line, int at, int c)
{
	if (at < 0 || at > line->size)
		at = line->size;

	line->chars = realloc(line->chars, line->size + 2);
	memmove(&line->chars[at + 1], &line->chars[at], line->size - at + 1);
	line->size++;
	line->chars[at] = c;

	editor_update_line(editor, line);

	editor->dirty = 1;
}

void line_append_string(struct editor_state *editor, line_t *line, char* string, size_t length)
{
	line->chars = realloc(line->chars, line->size + length + 1);
	memcpy(&line->chars[line->size], string, length);
	line->size += length;
	line->chars[line->size] = '\0';

	editor_update_line(editor, line);
	editor->dirty = 1;
}

void line_delete_char(struct editor_state *editor, line_t *line, int at)
{
	if (at < 0 || at >= line->size)
		return;

	memmove(&line->chars[at], &line->chars[at + 1], line->size - at);
	line->size--;
	editor_update_line(editor, line);
	editor->dirty = 1;
}
