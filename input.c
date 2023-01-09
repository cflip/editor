#include "input.h"

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "file.h"
#include "terminal.h"
#include "row.h"

int editor_read_key()
{
	int read_count;
	char c;

	while ((read_count = read(STDIN_FILENO, &c, 1)) != 1) {
		if (read_count == -1 && errno != EAGAIN)
			die("read");
	}

	if (c == '\x1b') {
		char seq[3];

		if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
		if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

		if (seq[0] == '[') {
			if (seq[1] >= '0' && seq[1] <= '9') {
				if (read(STDIN_FILENO, &seq[2], 1) != 1)
					return '\x1b';

				if (seq[2] == '~') {
					switch (seq[1]) {
						case '1': return HOME_KEY;
						case '3': return DELETE_KEY;
						case '4': return END_KEY;
						case '5': return PAGE_UP;
						case '6': return PAGE_DOWN;
						case '7': return HOME_KEY;
						case '8': return END_KEY;
					}
				}
			} else {
				switch (seq[1]) {
					case 'A': return ARROW_UP;
					case 'B': return ARROW_DOWN;
					case 'C': return ARROW_RIGHT;
					case 'D': return ARROW_LEFT;
					case 'H': return HOME_KEY;
					case 'F': return END_KEY;
				}
			}
		} else if (seq[0] == 'O') {
			switch (seq[1]) {
				case 'H': return HOME_KEY;
				case 'F': return END_KEY;
			}
		}
		return '\x1b';
	} else {
		return c;
	}
}

void editor_move_cursor(struct editor_state* editor, int key)
{
	struct editor_row* row = (editor->cursor_y >= editor->row_count) ? NULL : &editor->rows[editor->cursor_y];
	
	switch (key) {
		case ARROW_LEFT:
			if (editor->cursor_x != 0) {
				editor->cursor_x--;
			} else if (editor->cursor_y > 0) {
				editor->cursor_y--;
				editor->cursor_x = editor->rows[editor->cursor_y].size;
			}
			break;
		case ARROW_RIGHT:
			if (row && editor->cursor_x < row->size) {
				editor->cursor_x++;
			} else if (row && editor->cursor_x == row->size) {
				editor->cursor_y++;
				editor->cursor_x = 0;
			}
			break;
		case ARROW_UP:
			if (editor->cursor_y != 0) editor->cursor_y--;
			break;
		case ARROW_DOWN:
			if (editor->cursor_y != editor->row_count - 1) editor->cursor_y++;
			break;
	}

	row = (editor->cursor_y >= editor->row_count) ? NULL : &editor->rows[editor->cursor_y];
	int row_length = row ? row->size : 0;
	if (editor->cursor_x > row_length)
		editor->cursor_x = row_length;
}

void editor_process_keypress(struct editor_state* editor, int c)
{
	static int quit_message = 1;

	switch (c) {
		case '\r':
			editor_insert_newline(editor);
			break;
		case CTRL_KEY('q'):
			if (editor->dirty && quit_message) {
				editor_set_status_message(editor, "This file has unsaved changes. Press Ctrl+Q again to quit");
				quit_message = 0;
				return;
			}
			exit(0);
			break;
		case CTRL_KEY('s'):
			editor_save(editor);
			break;
		case HOME_KEY:
			editor->cursor_x = 0;
			break;
		case END_KEY:
			if (editor->cursor_y < editor->row_count)
				editor->cursor_x = editor->rows[editor->cursor_y].size;
			break;
		case CTRL_KEY('f'):
			editor_find(editor);
			break;
		case BACKSPACE:
		case DELETE_KEY:
			if (c == DELETE_KEY) 
				editor_move_cursor(editor, ARROW_RIGHT);
			editor_delete_char(editor);
			break;
		case PAGE_UP:
		case PAGE_DOWN:
			{
				if (c == PAGE_UP) {
					editor->cursor_y = editor->row_offset;
				} else {
					editor->cursor_y = editor->row_offset + editor->screen_rows -1;
					if (editor->cursor_y > editor->row_count)
						editor->cursor_y = editor->row_count;
				}

				int times = editor->screen_rows;
				while (times--)
					editor_move_cursor(editor, c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
			}
			break;
		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			editor_move_cursor(editor, c);
			break;
		case CTRL_KEY('l'):
		case '\x1b':
			break;
		default:
			editor_insert_char(editor, c);
			break;
	}

	quit_message = 1;
}
