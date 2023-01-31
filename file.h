#ifndef _FILE_H
#define _FILE_H

#include "editor.h"

void editor_open(struct editor_state* editor, char* filename);
int file_save_current_file(struct editor_state *editor);

#endif
