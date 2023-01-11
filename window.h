#ifndef _WINDOW_H
#define _WINDOW_H

struct editor_state;

void window_init(const char *title, int rows, int cols);
int window_handle_event(struct editor_state *editor);
void window_redraw(struct editor_state *editor);
void window_get_size(int *rows, int *cols);
void window_destroy();

#endif
