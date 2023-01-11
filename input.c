#include "input.h"

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "file.h"
#include "row.h"

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
