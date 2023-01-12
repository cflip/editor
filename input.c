#include "input.h"

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "file.h"
#include "row.h"

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
				editor_move_right(editor);
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
				/*
				 * TODO: Reimplement pageup/pagedown, this time by scrolling the
				 * screen without necessarily changing the position of the
				 * cursor relative to the screen
				 */
			}
			break;
		case ARROW_UP:
			editor_move_up(editor);
			break;
		case ARROW_DOWN:
			editor_move_down(editor);
			break;
		case ARROW_LEFT:
			editor_move_left(editor);
			break;
		case ARROW_RIGHT:
			editor_move_right(editor);
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
