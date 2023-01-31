#include "editor.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "file.h"
#include "input.h"
#include "syntax.h"
#include "window.h"

/* How long a message stays on screen */
#define MESSAGE_TIMEOUT_SECONDS 5

/* How long the user has to press Ctrl+Q again to quit an unsaved file. */
#define QUIT_TIMEOUT_SECONDS 5

static int quit_message_time = 0;

void init_editor(struct editor_state* editor)
{
	editor->cursor_x = 0;
	editor->cursor_y = 0;
	editor->cursor_display_x = 0;
	editor->line_offset = 0;
	editor->col_offset = 0;
	editor->num_lines = 0;
	editor->lines = NULL;
	editor->dirty = 0;
	editor->filename = NULL;
	editor->status_message[0] = '\0';
	editor->status_message_time = 0;
	editor->syntax = NULL;
	editor->mode = EDITOR_MODE_NORMAL;
	editor->cmdline = textbuf_init();

	editor_update_screen_size(editor);
	window_set_filename("[New]");
}

void editor_set_status_message(struct editor_state* editor, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(editor->status_message, sizeof(editor->status_message), format, args);
	va_end(args);
	editor->status_message_time = time(NULL);
}

static prompt_callback_t saved_prompt_callback;

void editor_prompt(struct editor_state* editor, char* prompt, prompt_callback_t callback)
{
	saved_prompt_callback = callback;
	editor_set_mode(editor, EDITOR_MODE_PROMPT);
}

void editor_run_command(struct editor_state *editor)
{
	textbuf_append(&editor->cmdline, "\0", 1);

	if (saved_prompt_callback) {
		saved_prompt_callback(editor, editor->cmdline.buffer, editor->cmdline.length);
	}

	/* TODO: Parse and run a command by its name */
	editor_set_mode(editor, EDITOR_MODE_NORMAL);
}

static void save_callback(struct editor_state *editor, char *filename, size_t namelen)
{
	if (filename == NULL)
		return;

	editor->filename = malloc(namelen);
	memcpy(editor->filename, filename, namelen);

	editor_select_syntax_highlight(editor);
	window_set_filename(editor->filename);

	editor_try_save(editor);
}

void editor_try_save(struct editor_state *editor)
{
	/*
	 * If there is no filename, set a callback to change the filename and
	 * come back here later once it is set.
	 */
	if (editor->filename == NULL) {
		editor_prompt(editor, "Save as: ", save_callback);
		return;
	}

	int saverr = file_save_current_file(editor);
	if (saverr != 0)
		editor_set_status_message(editor, "Failed to save file: %s", strerror(saverr));
}

void editor_try_quit(struct editor_state *editor)
{
	if (editor->dirty && quit_message_time == 0) {
		editor_set_status_message(editor, "This file has unsaved changes. Press Ctrl+Q again to quit");
		quit_message_time = time(NULL);
		return;
	}
	exit(0);
}

void editor_move_left(struct editor_state *editor)
{
	if (editor->cursor_x != 0) {
		editor->cursor_x--;
	} else if (editor->cursor_y > 0) {
		editor->cursor_y--;
		editor->cursor_x = editor->lines[editor->cursor_y].size;
	}
}

void editor_move_right(struct editor_state *editor)
{
	line_t *line = (editor->cursor_y >= editor->num_lines) ? NULL : &editor->lines[editor->cursor_y];
	if (line && editor->cursor_x < line->size) {
		editor->cursor_x++;
	} else if (line && editor->cursor_x == line->size) {
		editor->cursor_y++;
		editor->cursor_x = 0;
	}
}

void editor_move_up(struct editor_state *editor)
{
	if (editor->cursor_y != 0)
		editor->cursor_y--;

	line_t *line = (editor->cursor_y >= editor->num_lines) ? NULL : &editor->lines[editor->cursor_y];
	int line_length = line ? line->size : 0;
	if (editor->cursor_x > line_length)
		editor->cursor_x = line_length;
}

void editor_move_down(struct editor_state *editor)
{
	if (editor->cursor_y != editor->num_lines - 1)
		editor->cursor_y++;

	line_t *line = (editor->cursor_y >= editor->num_lines) ? NULL : &editor->lines[editor->cursor_y];
	int line_length = line ? line->size : 0;
	if (editor->cursor_x > line_length)
		editor->cursor_x = line_length;
}

void editor_move_end(struct editor_state *editor)
{
	if (editor->cursor_y < editor->num_lines)
		editor->cursor_x = editor->lines[editor->cursor_y].size;
}

void editor_insert_char(struct editor_state* editor, int c)
{
	if (editor->cursor_y == editor->num_lines)
		editor_insert_line(editor, editor->num_lines, "", 0);

	line_insert_char(editor, &editor->lines[editor->cursor_y], editor->cursor_x, c);
	editor->cursor_x++;
}

void editor_insert_newline(struct editor_state* editor)
{
	if (editor->cursor_x == 0) {
		editor_insert_line(editor, editor->cursor_y, "", 0);
	} else {
		line_t *line = &editor->lines[editor->cursor_y];
		editor_insert_line(editor, editor->cursor_y + 1, &line->chars[editor->cursor_x], line->size - editor->cursor_x);
		line = &editor->lines[editor->cursor_y];
		line->size = editor->cursor_x;
		line->chars[line->size] = '\0';
		editor_update_line(editor, line);
	}
	editor->cursor_y++;
	editor->cursor_x = 0;
}

void editor_delete_char(struct editor_state* editor)
{
	if (editor->cursor_y == editor->num_lines)
		return;

	if (editor->cursor_x == 0 && editor->cursor_y == 0)
		return;

	line_t *line = &editor->lines[editor->cursor_y];
	if (editor->cursor_x > 0) {
		line_delete_char(editor, line, editor->cursor_x - 1);
		editor->cursor_x--;
	} else {
		editor->cursor_x = editor->lines[editor->cursor_y - 1].size;
		line_append_string(editor, &editor->lines[editor->cursor_y - 1], line->chars, line->size);
		editor_delete_line(editor, editor->cursor_y);
		editor->cursor_y--;
	}
}

void editor_add_line_above(struct editor_state* editor)
{
	editor_insert_line(editor, editor->cursor_y, "", 0);
	editor->cursor_x = 0;
}

void editor_add_line_below(struct editor_state* editor)
{
	editor_insert_line(editor, editor->cursor_y + 1, "", 0);
	editor->cursor_y++;
	editor->cursor_x = 0;
}

void editor_set_mode(struct editor_state *editor, enum editor_mode mode)
{
	enum editor_mode last_mode = editor->mode;
	if (mode == last_mode)
		return;

	/* Clear the command line if we are leaving prompt mode. */
	if (last_mode == EDITOR_MODE_PROMPT) {
		textbuf_clear(&editor->cmdline);
		saved_prompt_callback = NULL;
	}

	/* Ignore the extra first letter if we are entering a typing mode. */
	if (mode != EDITOR_MODE_NORMAL) {
		editor->pressed_insert_key = 1;
	}

	editor->mode = mode;
}

static void editor_find_callback(struct editor_state* editor, char* query, int key)
{
	static int last_match = -1;
	static int direction = 1;

	static int saved_highlight_line;
	static char* saved_highlight;

	if (saved_highlight) {
		memset(editor->lines[saved_highlight_line].highlight, (size_t)saved_highlight, editor->lines[saved_highlight_line].render_size);
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
	for (i = 0; i < editor->num_lines; i++) {
		current += direction;
		if (current == -1) {
			current = editor->num_lines - 1;
		} else if (current == editor->num_lines) {
			current = 0;
		}

		line_t *line = &editor->lines[current];
		char* match = strstr(line->render, query);
		
		if (match) {
			last_match = current;
			editor->cursor_y = current;
			editor->cursor_x = row_display_x_to_x(line, match - line->render);
			editor->line_offset = editor->num_lines;

			saved_highlight_line = current;
			saved_highlight = malloc(line->render_size);
			memcpy(saved_highlight, line->highlight, line->render_size);
			memset(&line->highlight[match - line->render], HIGHLIGHT_MATCH, strlen(query));
			break;
		}
	}
}

void editor_find(struct editor_state* editor)
{
	/* TODO: Unimplemented */
}

void editor_scroll(struct editor_state* editor)
{
	editor->cursor_display_x = 0;
	if (editor->cursor_y < editor->num_lines)
		editor->cursor_display_x = row_x_to_display_x(&editor->lines[editor->cursor_y], editor->cursor_x);

	if (editor->cursor_y < editor->line_offset)
		editor->line_offset = editor->cursor_y;

	if (editor->cursor_y >= editor->line_offset + editor->screen_rows)
		editor->line_offset = editor->cursor_y - editor->screen_rows + 1;

	if (editor->cursor_display_x < editor->col_offset)
		editor->col_offset = editor->cursor_display_x;

	if (editor->cursor_display_x >= editor->col_offset + editor->screen_cols)
		editor->col_offset = editor->cursor_display_x - editor->screen_cols + 1;
}

void editor_update_screen_size(struct editor_state *editor)
{
	window_get_size(&editor->screen_rows, &editor->screen_cols);
	editor->screen_rows -= 2;
}

void editor_draw_status_bar(struct editor_state* editor, struct textbuf *buffer)
{
	char status[80], right_status[80];
	int length = snprintf(status, sizeof(status), "%.20s - %d lines %s", editor->filename ? editor->filename : "[New File]", editor->num_lines, editor->dirty ? "(modified)" : "");
	int right_length = snprintf(right_status, sizeof(right_status), "%s | %d/%d", editor->syntax ? editor->syntax->filetype : "plaintext", editor->cursor_y + 1, editor->num_lines);

	if (length > editor->screen_cols)
		length = editor->screen_cols;

	textbuf_append(buffer, status, length);
	
	while (length < editor->screen_cols) {
		if (editor->screen_cols - length == right_length) {
			textbuf_append(buffer, right_status, right_length);
			break;
		} else {
			textbuf_append(buffer, " ", 1);
			length++;
		}
	}
	textbuf_append(buffer, "\n", 1);
}

void editor_draw_message_bar(struct editor_state* editor, struct textbuf *buffer)
{
	if (editor->mode == EDITOR_MODE_INSERT) {
		textbuf_append(buffer, "--INSERT--", 10);
		return;
	}

	if (editor->mode == EDITOR_MODE_PROMPT) {
		textbuf_append(buffer, editor->cmdline.buffer, editor->cmdline.length);
		return;
	}

	int message_length = strlen(editor->status_message);
	
	if (message_length > editor->screen_cols)
		message_length = editor->screen_cols;

	if (message_length && time(NULL) - editor->status_message_time < MESSAGE_TIMEOUT_SECONDS)
		textbuf_append(buffer, editor->status_message, message_length);

	if (time(NULL) - quit_message_time > QUIT_TIMEOUT_SECONDS)
		quit_message_time = 0;
}

void editor_destroy(struct editor_state *editor)
{
	free(editor->filename);
	for (int i = 0; i < editor->num_lines; i++)
		free_line(&editor->lines[i]);
	free(editor->lines);
	textbuf_free(&editor->cmdline);
}
