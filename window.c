#include "window.h"

#include <SDL2/SDL.h>

#include "buffer.h"
#include "editor.h"
#include "error.h"
#include "font.h"
#include "input.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *font_texture = NULL;

static int window_width;
static int window_height;

static PSFFont font;

void window_init(const char *title, int rows, int cols)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		fatal_error("Failed to init SDL: %s\n", SDL_GetError());

	font = font_load("terminus/ter-u24n.psf");
	window_width = cols * font.width;
	window_height = rows * font.height;

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_RESIZABLE);
	if (window == NULL)
		fatal_error("Failed to create window: %s\n", SDL_GetError());

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL)
		fatal_error("Failed to create renderer: %s\n", SDL_GetError());

	font_texture = font_create_texture(renderer, &font);

	SDL_ShowWindow(window);
}

int window_handle_event(struct editor_state *editor)
{
	static SDL_Event e;
	SDL_WaitEvent(&e);
	switch (e.type) {
	case SDL_QUIT:
		return 0;
	case SDL_KEYDOWN:
		editor_process_keypress(editor, &e.key.keysym);
		break;
	case SDL_TEXTINPUT:
		if (editor->mode == EDITOR_MODE_NORMAL)
			break;
		/*
		 * Ignore the first letter after entering insert mode, because it
		 * usually is just 'i'.
		 */
		if (editor->pressed_insert_key) {
			editor->pressed_insert_key = 0;
			break;
		}
		editor_insert_char(editor, *e.text.text);
		break;
	case SDL_WINDOWEVENT:
		if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			SDL_GetWindowSize(window, &window_width, &window_height);
			editor_update_screen_size(editor);
		}
		break;
	}
	return 1;
}

static void draw_string(const char *str, size_t len, int x, int y)
{
	SDL_Rect dstrect = { x, y, font.width, font.height };
	SDL_Rect srcrect = { 0, 0, font.width, font.height };

	for (int i = 0; i < len; i++) {
		const char letter = str[i];

		if (letter == '\n') {
			dstrect.x = x;
			dstrect.y += font.height;
			continue;
		}

		if (isspace(letter)) {
			dstrect.x += font.width;
			continue;
		}

		if (letter == '\0')
			break;

		int glyph_index = letter;
		if (font.unicode_desc != NULL)
			glyph_index = font.unicode_desc[glyph_index];
		srcrect.x = glyph_index * font.width;

		SDL_RenderCopy(renderer, font_texture, &srcrect, &dstrect);

		dstrect.x += font.width;
	}
}

static void draw_editor(struct editor_state *editor)
{
	int line_y;

	/* Draw each line of text. */
	for (int i = 0; i < editor->screen_rows; i++) {
		line_y = i * font.height;

		if (i + editor->line_offset >= editor->num_lines) {
			draw_string("~", 1, 0, line_y);
			continue;
		}

		line_t *line = &editor->lines[i + editor->line_offset];

		/* Size and length of the text including the scroll offset. */
		char *printed_text = &line->render[editor->col_offset];
		size_t printed_size = line->render_size - editor->col_offset;
		if (line->render_size >= printed_size)
			draw_string(printed_text, printed_size, 0, line_y);
	}

	/* Draw the statusline containing file information */
	struct append_buffer statusbuf = ABUF_INIT;

	editor_draw_status_bar(editor, &statusbuf);
	editor_draw_message_bar(editor, &statusbuf);

	line_y = window_height - (font.height * 2);
	draw_string(statusbuf.buffer, statusbuf.length, 0, line_y);

	ab_free(&statusbuf);
}

void window_redraw(struct editor_state *editor)
{
	SDL_RenderClear(renderer);

	editor_scroll(editor);
	draw_editor(editor);

	SDL_Rect cursor_rect;
	cursor_rect.x = (editor->cursor_display_x - editor->col_offset) * font.width;
	cursor_rect.y = (editor->cursor_y - editor->line_offset) * font.height;
	cursor_rect.w = font.width;
	cursor_rect.h = font.height;

	SDL_SetRenderDrawColor(renderer, 0x7f, 0x7f, 0x7f, 0xff);
	SDL_RenderFillRect(renderer, &cursor_rect);
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);

	SDL_RenderPresent(renderer);
	SDL_UpdateWindowSurface(window);
}

void window_get_size(int *rows, int *cols)
{
	*cols = window_width / font.width;
	*rows = window_height / font.height;
}

void window_destroy()
{
	font_destroy(&font);

	SDL_DestroyTexture(font_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
