#ifndef _SYNTAX_H
#define _SYNTAX_H

#include <stdlib.h>

#include "editor.h"
#include "line.h"

#define HIGHLIGHT_FLAG_NUMBERS (1 << 0)
#define HIGHLIGHT_FLAG_STRINGS (1 << 1)

struct editor_syntax {
	char* filetype;
	char** filetype_match;
	char** keywords;
	char* single_line_comment_start;
	char* multi_line_comment_start;
	char* multi_line_comment_end;
	int flags;
};

enum editor_highlight {
	HIGHLIGHT_NORMAL = 0,
	HIGHLIGHT_COMMENT,
	HIGHLIGHT_MULTILINE_COMMENT,
	HIGHLIGHT_KEYWORD1,
	HIGHLIGHT_KEYWORD2,
	HIGHLIGHT_STRING,
	HIGHLIGHT_NUMBER,
	HIGHLIGHT_MATCH
};

#define HIGHLIGHT_DATABASE_ENTRY_COUNT (sizeof(highlight_database) / sizeof(highlight_database[0]))

void editor_update_syntax(struct editor_state* editor, line_t*);
int editor_syntax_to_colour(int highlight);
void editor_select_syntax_highlight(struct editor_state* editor);

#endif
