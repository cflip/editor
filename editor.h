#ifndef _EDITOR_H
#define _EDITOR_H

#include <time.h>
#include "buffer.h"

enum editor_mode {
	EDITOR_MODE_NORMAL,
	EDITOR_MODE_INSERT,
	EDITOR_MODE_COMMAND
};

struct editor_state {
	int cursor_x, cursor_y;
	int cursor_display_x;
	int row_offset;
	int col_offset;
	int screen_rows;
	int screen_cols;
	int row_count;
	struct editor_row* rows;
	int dirty;
	char* filename;
	char status_message[80];
	time_t status_message_time;
	struct editor_syntax* syntax;
	int mode;
	/*
	 * Keep track of whether a key that toggles insert mode has been pressed, so
	 * we can ignore it during the text input event. Otherwise, an 'i' or other
	 * letter will be inserted when entering insert mode.
	 */
	int pressed_insert_key;
};

void init_editor(struct editor_state* editor);

void editor_set_status_message(struct editor_state* editor, const char* format, ...);
char* editor_prompt(struct editor_state* editor, char* prompt, void (*callback)(struct editor_state*, char*, int));

void editor_move_left(struct editor_state *);
void editor_move_right(struct editor_state *);
void editor_move_up(struct editor_state *);
void editor_move_down(struct editor_state *);

void editor_insert_char(struct editor_state* editor, int c);
void editor_insert_newline(struct editor_state* editor);
void editor_delete_char(struct editor_state* editor);
void editor_add_line_above(struct editor_state* editor);
void editor_add_line_below(struct editor_state* editor);

void editor_find(struct editor_state* editor);
void editor_scroll(struct editor_state* editor);
void editor_draw_rows(struct editor_state* editor, struct append_buffer* buffer);
void editor_draw_status_bar(struct editor_state* editor, struct append_buffer* buffer);
void editor_draw_message_bar(struct editor_state* editor, struct append_buffer* buffer);

void editor_destroy(struct editor_state *editor);

#endif
