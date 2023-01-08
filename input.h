#pragma once

#include "editor.h"

enum editor_key {
	BACKSPACE = 127,
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DELETE_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN
};

#define CTRL_KEY(k) ((k) & 0x1f)

int editor_read_key();
void editor_move_cursor(struct editor_state* editor, int key);
void editor_process_keypress(struct editor_state* editor);
