#include "input.h"
#include "file.h"
#include "editor.h"
#include "terminal.h"

int main(int argc, char** argv)
{
	enable_raw_mode();

	struct editor_state editor;
	init_editor(&editor);

	if (argc >= 2) {
		editor_open(&editor, argv[1]);
	}

	editor_set_status_message(&editor, "HELP: Ctrl+Q = quit, Ctrl+S = save, Ctrl+F = find");

	while (1) {
		editor_refresh_screen(&editor);
		editor_process_keypress(&editor);
	}
	
	return 0;
}
