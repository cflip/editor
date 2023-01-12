#include "input.h"

#include "editor.h"
#include "row.h"

void editor_process_keypress(struct editor_state *editor, SDL_Keysym *keysym)
{
	static int quit_message = 1;

	/* Handle keypresses for typing modes separately. */
	if (editor->mode != EDITOR_MODE_NORMAL) {
		if (keysym->sym == SDLK_BACKSPACE)
			editor_delete_char(editor);

		if (keysym->sym == SDLK_RETURN)
			editor_insert_newline(editor);

		if (keysym->sym == SDLK_ESCAPE)
			editor->mode = EDITOR_MODE_NORMAL;
		return;
	}

	switch (keysym->sym) {
		case '\r':
			editor_insert_newline(editor);
			break;
		case SDLK_q:
			if (keysym->mod & KMOD_CTRL) {
				if (editor->dirty && quit_message) {
					editor_set_status_message(editor, "This file has unsaved changes. Press Ctrl+Q again to quit");
					quit_message = 0;
					return;
				}
				exit(0);
			}
			break;
		case SDLK_s:
			if (keysym->mod & KMOD_CTRL)
				editor_save(editor);
			break;
		case SDLK_0:
			editor->cursor_x = 0;
			break;
		case SDLK_4:
			if (keysym->mod & KMOD_SHIFT && editor->cursor_y < editor->row_count)
				editor->cursor_x = editor->rows[editor->cursor_y].size;
			break;
		case SDLK_SLASH:
			editor_find(editor);
			break;
		case SDLK_x:
			editor_move_right(editor);
			editor_delete_char(editor);
			break;
		case SDLK_PAGEUP:
		case SDLK_PAGEDOWN:
			/* TODO: Reimplement page up & page down. */
			break;
		case SDLK_i:
			editor->mode = EDITOR_MODE_INSERT;
			break;
		case SDLK_k:
			editor_move_up(editor);
			break;
		case SDLK_j:
			editor_move_down(editor);
			break;
		case SDLK_h:
			editor_move_left(editor);
			break;
		case SDLK_l:
			editor_move_right(editor);
			break;
	}

	quit_message = 1;
}
