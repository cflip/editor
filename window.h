#ifndef _WINDOW_H
#define _WINDOW_H

struct editor_state;

void window_init();
int window_handle_event(struct editor_state *editor);
void window_redraw(struct editor_state *editor);
void window_destroy();

#endif
