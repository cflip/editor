#include "input.h"
#include "file.h"
#include "editor.h"
#include "window.h"

int main(int argc, char** argv)
{
	window_init();

	struct editor_state editor;
	init_editor(&editor);

	if (argc >= 2) {
		editor_open(&editor, argv[1]);
	}

	editor_set_status_message(&editor, "HELP: Ctrl+Q = quit, Ctrl+S = save, Ctrl+F = find");

	while (window_handle_event(&editor)) {
		window_redraw(&editor);
		// editor_refresh_screen(&editor);
		// editor_process_keypress(&editor);
	}
	
	window_destroy();
	return 0;
}
