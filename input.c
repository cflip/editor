#include "input.h"

#include "editor.h"
#include "file.h"
#include "line.h"

void input_process_textinput(struct editor_state *editor, const char *text)
{
	/* Ignore the first letter after entering insert mode. */
	if (editor->pressed_insert_key) {
		editor->pressed_insert_key = 0;
		return;
	}

	if (editor->mode == EDITOR_MODE_INSERT) {
		editor_insert_char(editor, *text);
	} else if (editor->mode == EDITOR_MODE_COMMAND) {
		textbuf_append(&editor->cmdline, text, 1);
	}
}

void editor_process_keypress(struct editor_state *editor, SDL_Keysym *keysym)
{
	/* Handle keypresses for typing modes separately. */
	if (editor->mode == EDITOR_MODE_INSERT) {
		if (keysym->sym == SDLK_BACKSPACE)
			editor_delete_char(editor);

		if (keysym->sym == SDLK_RETURN)
			editor_insert_newline(editor);

		if (keysym->sym == SDLK_TAB)
			editor_insert_char(editor, '\t');

		if (keysym->sym == SDLK_ESCAPE)
			editor->mode = EDITOR_MODE_NORMAL;
		return;
	}

	if (editor->mode == EDITOR_MODE_COMMAND) {
		if (keysym->sym == SDLK_BACKSPACE)
			textbuf_delete(&editor->cmdline);

		if (keysym->sym == SDLK_RETURN)
			editor_run_command(editor);

		if (keysym->sym == SDLK_ESCAPE)
			editor->mode = EDITOR_MODE_NORMAL;
		return;
	}

	switch (keysym->sym) {
		/* TODO: Reimplement page up/down on Shift+W/S. */
		case SDLK_w:
			editor_move_up(editor);
			break;
		case SDLK_s:
			if (keysym->mod & KMOD_CTRL) {
				editor_save(editor);
				break;
			}
			editor_move_down(editor);
			break;
		case SDLK_a:
			editor_move_left(editor);
			break;
		case SDLK_d:
			editor_move_right(editor);
			break;
		case SDLK_q:
			if (keysym->mod & KMOD_CTRL) {
				editor_try_quit(editor);
				break;
			}
			editor->cursor_x = 0;
			break;
		case SDLK_e:
			editor_move_end(editor);
			break;
		case SDLK_n:
			editor_delete_char(editor);
			break;
		case SDLK_m:
			editor_move_right(editor);
			editor_delete_char(editor);
			break;
		case SDLK_i:
			editor->mode = EDITOR_MODE_INSERT;
			editor->pressed_insert_key = 1;
			break;
		case SDLK_l:
			if (keysym->mod & KMOD_SHIFT)
				editor_move_end(editor);
			else
				editor_move_right(editor);
			editor->mode = EDITOR_MODE_INSERT;
			editor->pressed_insert_key = 1;
			break;
		case SDLK_o:
			if (keysym->mod & KMOD_SHIFT)
				editor_add_line_above(editor);
			else
				editor_add_line_below(editor);
			editor->mode = EDITOR_MODE_INSERT;
			editor->pressed_insert_key = 1;
			break;
		case SDLK_SLASH:
			editor_find(editor);
			break;
		case SDLK_SEMICOLON:
			editor->mode = EDITOR_MODE_COMMAND;
			break;
	}
}
