#include "editor.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "input.h"
#include "syntax.h"
#include "row.h"

void init_editor(struct editor_state* editor)
{
	editor->cursor_x = 0;
	editor->cursor_y = 0;
	editor->cursor_display_x = 0;
	editor->row_offset = 0;
	editor->col_offset = 0;
	editor->row_count = 0;
	editor->rows = NULL;
	editor->dirty = 0;
	editor->filename = NULL;
	editor->status_message[0] = '\0';
	editor->status_message_time = 0;
	editor->syntax = NULL;
	editor->mode = EDITOR_MODE_NORMAL;

	window_get_size(&editor->screen_rows, &editor->screen_cols);

	editor->screen_rows -= 2;
}

void editor_set_status_message(struct editor_state* editor, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(editor->status_message, sizeof(editor->status_message), format, args);
	va_end(args);
	editor->status_message_time = time(NULL);
}

char* editor_prompt(struct editor_state* editor, char* prompt, void (*callback)(struct editor_state*, char*, int))
{
	/* TODO: The previous implementation of this function relied on reading
	 * input from the terminal, but now that we get input through window events
	 * it's no longer possible to sit in an infinite loop waiting for keys here.
	 */
	printf("TODO: editor_prompt unimplemented\n");
	return NULL;
}

void editor_move_left(struct editor_state *editor)
{
	if (editor->cursor_x != 0) {
		editor->cursor_x--;
	} else if (editor->cursor_y > 0) {
		editor->cursor_y--;
		editor->cursor_x = editor->rows[editor->cursor_y].size;
	}
}

void editor_move_right(struct editor_state *editor)
{
	struct editor_row* row = (editor->cursor_y >= editor->row_count) ? NULL : &editor->rows[editor->cursor_y];
	if (row && editor->cursor_x < row->size) {
		editor->cursor_x++;
	} else if (row && editor->cursor_x == row->size) {
		editor->cursor_y++;
		editor->cursor_x = 0;
	}
}

void editor_move_up(struct editor_state *editor)
{
	if (editor->cursor_y != 0)
		editor->cursor_y--;

	struct editor_row *row = (editor->cursor_y >= editor->row_count) ? NULL : &editor->rows[editor->cursor_y];
	int row_length = row ? row->size : 0;
	if (editor->cursor_x > row_length)
		editor->cursor_x = row_length;
}

void editor_move_down(struct editor_state *editor)
{
	if (editor->cursor_y != editor->row_count - 1)
		editor->cursor_y++;

	struct editor_row *row = (editor->cursor_y >= editor->row_count) ? NULL : &editor->rows[editor->cursor_y];
	int row_length = row ? row->size : 0;
	if (editor->cursor_x > row_length)
		editor->cursor_x = row_length;
}

void editor_insert_char(struct editor_state* editor, int c)
{
	if (editor->cursor_y == editor->row_count)
		insert_row(editor, editor->row_count, "", 0);

	row_insert_char(editor, &editor->rows[editor->cursor_y], editor->cursor_x, c);
	editor->cursor_x++;
}

void editor_insert_newline(struct editor_state* editor)
{
	if (editor->cursor_x == 0) {
		insert_row(editor, editor->cursor_y, "", 0);
	} else {
		struct editor_row* row = &editor->rows[editor->cursor_y];
		insert_row(editor, editor->cursor_y + 1, &row->chars[editor->cursor_x], row->size - editor->cursor_x);
		row = &editor->rows[editor->cursor_y];
		row->size = editor->cursor_x;
		row->chars[row->size] = '\0';
		update_row(editor, row);
	}
	editor->cursor_y++;
	editor->cursor_x = 0;
}

void editor_delete_char(struct editor_state* editor)
{
	if (editor->cursor_y == editor->row_count)
		return;

	if (editor->cursor_x == 0 && editor->cursor_y == 0)
		return;

	struct editor_row* row = &editor->rows[editor->cursor_y];
	if (editor->cursor_x > 0) {
		row_delete_char(editor, row, editor->cursor_x - 1);
		editor->cursor_x--;
	} else {
		editor->cursor_x = editor->rows[editor->cursor_y - 1].size;
		row_append_string(editor, &editor->rows[editor->cursor_y - 1], row->chars, row->size);
		delete_row(editor, editor->cursor_y);
		editor->cursor_y--;
	}
}

static void editor_find_callback(struct editor_state* editor, char* query, int key)
{
	static int last_match = -1;
	static int direction = 1;

	static int saved_highlight_line;
	static char* saved_highlight;

	if (saved_highlight) {
		memset(editor->rows[saved_highlight_line].highlight, (size_t)saved_highlight, editor->rows[saved_highlight_line].render_size);
		free(saved_highlight);
		saved_highlight = NULL;
	}

	/* TODO:
	if (key == '\r' || key == '\x1b') {
		last_match = -1;
		direction = 1;
		return;
	} else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
		direction = 1;
	} else if (key == ARROW_LEFT || key == ARROW_UP) {
		direction = -1;
	} else {
		last_match = -1;
		direction = 1;
	}
	*/

	if (last_match == -1)
		direction = 1;
	int current = last_match;	

	int i;
	for (i = 0; i < editor->row_count; i++) {
		current += direction;
		if (current == -1) {
			current = editor->row_count - 1;
		} else if (current == editor->row_count) {
			current = 0;
		}

		struct editor_row* row = &editor->rows[current];
		char* match = strstr(row->render, query);
		
		if (match) {
			last_match = current;
			editor->cursor_y = current;
			editor->cursor_x = row_display_x_to_x(row, match - row->render);
			editor->row_offset = editor->row_count;

			saved_highlight_line = current;
			saved_highlight = malloc(row->render_size);
			memcpy(saved_highlight, row->highlight, row->render_size);
			memset(&row->highlight[match - row->render], HIGHLIGHT_MATCH, strlen(query));
			break;
		}
	}
}

void editor_find(struct editor_state* editor)
{
	int saved_cursor_x = editor->cursor_x;
	int saved_cursor_y = editor->cursor_y;
	int saved_col_offset = editor->col_offset;
	int saved_row_offset = editor->row_offset;	

	char* query = editor_prompt(editor, "Search: %s (Use Esc/Arrows/Enter)", editor_find_callback);
	if (query) {
		free(query);
	} else {
		editor->cursor_x = saved_cursor_x;
		editor->cursor_y = saved_cursor_y;
		editor->col_offset = saved_col_offset;
		editor->row_offset = saved_row_offset;
	}
}

void editor_scroll(struct editor_state* editor)
{
	editor->cursor_display_x = 0;
	if (editor->cursor_y < editor->row_count)
		editor->cursor_display_x = row_x_to_display_x(&editor->rows[editor->cursor_y], editor->cursor_x);

	if (editor->cursor_y < editor->row_offset)
		editor->row_offset = editor->cursor_y;

	if (editor->cursor_y >= editor->row_offset + editor->screen_rows)
		editor->row_offset = editor->cursor_y - editor->screen_rows + 1;

	if (editor->cursor_display_x < editor->col_offset)
		editor->col_offset = editor->cursor_display_x;

	if (editor->cursor_display_x >= editor->col_offset + editor->screen_cols)
		editor->col_offset = editor->cursor_display_x - editor->screen_cols + 1;
}

void editor_draw_rows(struct editor_state* editor, struct append_buffer* buffer)
{
	int y;
	for (y = 0; y < editor->screen_rows; y++) {
		int file_row = y + editor->row_offset;
		if (file_row >= editor->row_count) {
			if (editor->row_count == 0 && y == editor->screen_rows / 3) {
				char welcome[80];
				int welcome_length = snprintf(welcome, sizeof(welcome), "Welcome to cflip text editor");
				
				if (welcome_length > editor->screen_cols)
					welcome_length = editor->screen_cols;

				int padding = (editor->screen_cols - welcome_length) / 2;
				if (padding) {
					ab_append(buffer, "~", 1);
					padding--;
				}

				while (padding--)
					ab_append(buffer, " ", 1);

				ab_append(buffer, welcome, welcome_length);
			} else {
				ab_append(buffer, "~", 1);
			}
		} else {
			int length = editor->rows[file_row].render_size - editor->col_offset;

			if (length < 0) length = 0;
			if (length > editor->screen_cols) length = editor->screen_cols;

			char* c = &editor->rows[file_row].render[editor->col_offset];
			unsigned char* highlight = &editor->rows[file_row].highlight[editor->col_offset];
			int current_colour = -1;
			int j;

			for (j = 0; j < length; j++) {
				/*
				 * TODO: Move colour handling stuff to the window.
				int colour = editor_syntax_to_colour(highlight[j]);
				if (colour != current_colour) {
					current_colour = colour;
					char colour_buffer[16];
					int colour_buffer_length = snprintf(colour_buffer, sizeof(colour_buffer), "\x1b[%dm", colour);

					ab_append(buffer, colour_buffer, colour_buffer_length);
				}
				*/
				ab_append(buffer, &c[j], 1);
			}
		}
		ab_append(buffer, "\n", 1);
	}
}

void editor_draw_status_bar(struct editor_state* editor, struct append_buffer* buffer)
{
	char status[80], right_status[80];
	int length = snprintf(status, sizeof(status), "%.20s - %d lines %s", editor->filename ? editor->filename : "[New File]", editor->row_count, editor->dirty ? "(modified)" : "");
	int right_length = snprintf(right_status, sizeof(right_status), "%s | %d/%d", editor->syntax ? editor->syntax->filetype : "plaintext", editor->cursor_y + 1, editor->row_count);

	if (length > editor->screen_cols)
		length = editor->screen_cols;

	ab_append(buffer, status, length);
	
	while (length < editor->screen_cols) {
		if (editor->screen_cols - length == right_length) {
			ab_append(buffer, right_status, right_length);
			break;
		} else {
			ab_append(buffer, " ", 1);
			length++;
		}
	}
	ab_append(buffer, "\n", 1);
}

void editor_draw_message_bar(struct editor_state* editor, struct append_buffer* buffer)
{
	if (editor->mode == EDITOR_MODE_INSERT) {
		ab_append(buffer, "--INSERT--", 10);
		return;
	}

	int message_length = strlen(editor->status_message);
	
	if (message_length > editor->screen_cols)
		message_length = editor->screen_cols;

	if (message_length && time(NULL) - editor->status_message_time < 5)
		ab_append(buffer, editor->status_message, message_length);
}
