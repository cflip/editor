#include "syntax.h"

#include <string.h>
#include "editor.h"

char* c_highlight_extensions[] = { ".c", ".h", ".cpp", ".cc", NULL };

char* c_highlight_keywords[] = {
	"switch", "if", "while", "for", "break", "continue", "return", "else",
	"struct", "union", "typedef", "static", "enum", "class", "case",
	"#include", "#define", "#ifdef", "#ifndef",

	"int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
	"void|", NULL
};

struct editor_syntax highlight_database[] = {
	{
		"c",
		c_highlight_extensions,
		c_highlight_keywords,
		"//", "/*", "*/",
		HIGHLIGHT_FLAG_NUMBERS | HIGHLIGHT_FLAG_STRINGS
	}
};

int is_separator(int c)
{
	return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editor_update_syntax(struct editor_state* editor, struct editor_row* row)
{
	row->highlight = realloc(row->highlight, row->render_size);
	memset(row->highlight, HIGHLIGHT_NORMAL, row->render_size);

	if (editor->syntax == NULL)
		return;

	char** keywords = editor->syntax->keywords;

	// TODO: Remove
	char* single_line_comment_start = editor->syntax->single_line_comment_start;
	char* multi_line_comment_start = editor->syntax->multi_line_comment_start;
	char* multi_line_comment_end = editor->syntax->multi_line_comment_end;
	
	int single_line_comment_start_length = single_line_comment_start ? strlen(single_line_comment_start) : 0;
	int multi_line_comment_start_length = multi_line_comment_start ? strlen(multi_line_comment_start) : 0;
	int multi_line_comment_end_length = multi_line_comment_end ? strlen(multi_line_comment_end) : 0;

	int previous_separator = 1;
	int in_string = 0;
	int in_comment = (row->index > 0 && editor->rows[row->index - 1].highlight_open_comment);

	int i = 0;
	while (i < row->render_size) {
		char c = row->render[i];
		unsigned char previous_highlight = (i > 0) ? row->highlight[i - 1] : HIGHLIGHT_NORMAL;

		if (single_line_comment_start_length && !in_string && !in_comment) {
			if (!strncmp(&row->render[i], single_line_comment_start, single_line_comment_start_length)) {
				memset(&row->highlight[i], HIGHLIGHT_COMMENT, row->render_size - i);
				break;
			}			
		}

		if (multi_line_comment_start_length && multi_line_comment_end_length && !in_string) {
			if (in_comment) {
				row->highlight[i] = HIGHLIGHT_MULTILINE_COMMENT;
				if (!strncmp(&row->render[i], multi_line_comment_end, multi_line_comment_end_length)) {
					memset(&row->highlight[i], HIGHLIGHT_MULTILINE_COMMENT, multi_line_comment_end_length);

					i += multi_line_comment_end_length;
					in_comment = 0;
					previous_separator = 1;
					continue;
				} else {
					i++;
					continue;
				}
			} else if (!strncmp(&row->render[i], multi_line_comment_start, multi_line_comment_start_length)) {
				memset(&row->highlight[i], HIGHLIGHT_MULTILINE_COMMENT, multi_line_comment_start_length);
				i += multi_line_comment_start_length;
				in_comment = 1;
				continue;
			}
		}

		if (editor->syntax->flags & HIGHLIGHT_FLAG_STRINGS) {
			if (in_string) {
				row->highlight[i] = HIGHLIGHT_STRING;

				if (c == '\\' && i + 1 < row->render_size) {
					row->highlight[i + 1] = HIGHLIGHT_STRING;
					i += 2;
					continue;
				}

				if (c == in_string)
					in_string = 0;

				i++;
				previous_separator = 1;
				continue;
			} else {
				if (c == '"' || c == '\'') {
					in_string = c;
					row->highlight[i] = HIGHLIGHT_STRING;
					i++;
					continue;
				}
			}
		}

		if (editor->syntax->flags & HIGHLIGHT_FLAG_NUMBERS) {
			if ((isdigit(c) && (previous_separator || previous_highlight == HIGHLIGHT_NUMBER)) || (c == '.' && previous_highlight == HIGHLIGHT_NUMBER)) {
				row->highlight[i] = HIGHLIGHT_NUMBER;
				i++;
				previous_separator = 0;
				continue;
			}
		}

		if (previous_separator) {
			int j;
			for (j = 0; keywords[j]; j++) {
				int keyword_length = strlen(keywords[j]);
				int is_secondary = keywords[j][keyword_length - 1] == '|';

				if (is_secondary)
					keyword_length--;

				if (!strncmp(&row->render[i], keywords[j], keyword_length) && is_separator(row->render[i + keyword_length])) {
					memset(&row->highlight[i], is_secondary ? HIGHLIGHT_KEYWORD2 : HIGHLIGHT_KEYWORD1, keyword_length);
					i += keyword_length;
					break;
				}
			}
			if (keywords[j] != NULL) {
				previous_separator = 0;
				continue;
			}
		}

		previous_separator = is_separator(c);
		i++;
	}

	int changed = (row->highlight_open_comment != in_comment);
	row->highlight_open_comment = in_comment;
	if (changed && row->index + 1 < editor->row_count)
		editor_update_syntax(editor, &editor->rows[row->index + 1]);
}

int editor_syntax_to_colour(int highlight)
{
	switch (highlight) {
		case HIGHLIGHT_MULTILINE_COMMENT:
		case HIGHLIGHT_COMMENT: return 36;
		case HIGHLIGHT_KEYWORD1: return 33;
		case HIGHLIGHT_KEYWORD2: return 32;
		case HIGHLIGHT_STRING: return 35;
		case HIGHLIGHT_NUMBER: return 31;
		case HIGHLIGHT_MATCH: return 34;
		default: return 37;
	}
}

void editor_select_syntax_highlight(struct editor_state* editor)
{
	editor->syntax = NULL;
	
	if (editor->filename == NULL)
		return;

	char* extension = strrchr(editor->filename, '.');

	for (unsigned int j = 0; j < HIGHLIGHT_DATABASE_ENTRY_COUNT; j++) {
		struct editor_syntax* syntax = &highlight_database[j];
		unsigned int i = 0;

		while (syntax->filetype_match[i]) {
			int is_extension = (syntax->filetype_match[i][0] == '.');
			if ((is_extension && extension && !strcmp(extension, syntax->filetype_match[i])) || (!is_extension && strstr(editor->filename, syntax->filetype_match[i]))) {
				editor->syntax = syntax;

				int file_row;
				for (file_row = 0; file_row < editor->row_count; file_row++) {
					editor_update_syntax(editor, &editor->rows[file_row]);
				}
				
				return;
			}
			i++;
		}
	}
}
