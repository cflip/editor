#ifndef _INPUT_H
#define _INPUT_H

#include "editor.h"
#include <SDL2/SDL_keyboard.h>

void editor_process_keypress(struct editor_state *editor, SDL_Keysym *keysym);

#endif
