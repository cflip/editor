#include "file.h"

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "row.h"
#include "syntax.h"

char* editor_rows_to_string(struct editor_state* editor, int* buffer_length)
{
	int total_length = 0;
	int j;

	for (j = 0; j < editor->row_count; j++)
		total_length += editor->rows[j].size + 1;

	*buffer_length = total_length;

	char* buffer = malloc(total_length);
	char* p = buffer;

	for (j = 0; j < editor->row_count; j++) {
		memcpy(p, editor->rows[j].chars, editor->rows[j].size);
		p += editor->rows[j].size;
		*p = '\n';
		p++;
	}

	return buffer;
}

void editor_open(struct editor_state* editor, char* filename)
{
	free(editor->filename);
	size_t filename_len = strlen(filename) + 1;
	editor->filename = malloc(filename_len);
	memcpy(editor->filename, filename, filename_len);

	editor_select_syntax_highlight(editor);

	/* If there is no file with this name, the editor will create it on save. */
	if (access(filename, F_OK) != 0)
		return;

	FILE* fp = fopen(filename, "r");
	if (!fp) {
		fatal_error("Failed to read file from %s\n", filename);
	}

	char* line = NULL;
	size_t line_capacity = 0;
	ssize_t line_length;

	while ((line_length = getline(&line, &line_capacity, fp)) != -1) {
		while (line_length > 0 && (line[line_length - 1] == '\n' || line[line_length - 1] == '\r'))
			line_length--;

		insert_row(editor, editor->row_count, line, line_length);
	}

	free(line);
	fclose(fp);

	editor->dirty = 0;
}

void editor_save(struct editor_state* editor)
{
	if (editor->filename == NULL) {
		editor->filename = editor_prompt(editor, "Save as: %s", NULL);
		
		if (editor->filename == NULL)
			return;

		editor_select_syntax_highlight(editor);
	}

	int length;
	char* buffer = editor_rows_to_string(editor, &length);

	int fd = open(editor->filename, O_RDWR | O_CREAT, 0644);
	if (fd != -1) {
		if (ftruncate(fd, length) != -1) {
			if (write(fd, buffer, length) == length) {
				close(fd);
				free(buffer);
				editor_set_status_message(editor, "%d bytes written to disk", length);
				editor->dirty = 0;
				return;
			}
		}
		close(fd);
	}
	free(buffer);
	editor_set_status_message(editor, "Failed to write to disk: %s", strerror(errno));
}
