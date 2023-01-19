#include "file.h"
#include "editor.h"
#include "window.h"

int main(int argc, char** argv)
{
	window_init("Text editor", 28, 80);

	struct editor_state editor;
	init_editor(&editor);

	if (argc >= 2) {
		editor_open(&editor, argv[1]);
	}

	editor_set_status_message(&editor, "HELP: Ctrl+Q: quit, Ctrl+S: save");

	while (window_handle_event(&editor)) {
		window_redraw(&editor);
	}
	
	window_destroy();
	editor_destroy(&editor);

	return 0;
}
