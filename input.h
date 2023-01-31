#ifndef _INPUT_H
#define _INPUT_H

#include "editor.h"
#include <SDL2/SDL_keyboard.h>

void input_process_textinput(struct editor_state *editor, const char *text);
void editor_process_keypress(struct editor_state *editor, SDL_Keysym *keysym);

#endif
