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

static PSFFont font;

void window_init(const char *title, int rows, int cols)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		fatal_error("Failed to init SDL: %s\n", SDL_GetError());

	font = font_load("terminus/ter-u12n.psf");
	int window_width = cols * font.width;
	int window_height = rows * font.height;

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, 0);
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
		editor_insert_char(editor, *e.text.text);
		break;
	}
	return 1;
}

static void draw_font_text(struct append_buffer *buffer)
{
	SDL_Rect dstrect = { 0, 0, 0, 0 };
	SDL_Rect srcrect = { 0, 0, 0, 0 };

	for (int i = 0; i < buffer->length; i++) {
		const char letter = buffer->buffer[i];

		if (letter == ' ') {
			dstrect.x += font.width;
			continue;
		}

		if (letter == '\n') {
			dstrect.x = 0;
			dstrect.y += font.height;
			continue;
		}

		int glyph_index = letter;
		if (font.unicode_desc != NULL) {
			glyph_index = font.unicode_desc[glyph_index];
		}

		dstrect.w = font.width;
		dstrect.h = font.height;

		srcrect.x = glyph_index * 8;
		srcrect.y = 0;
		srcrect.w = font.width;
		srcrect.h = font.height;
		SDL_RenderCopy(renderer, font_texture, &srcrect, &dstrect);

		dstrect.x += font.width;
	}
}

void window_redraw(struct editor_state *editor)
{
	SDL_RenderClear(renderer);

	editor_scroll(editor);
	struct append_buffer buffer = ABUF_INIT;

	editor_draw_rows(editor, &buffer);
	editor_draw_status_bar(editor, &buffer);
	editor_draw_message_bar(editor, &buffer);

	draw_font_text(&buffer);

	SDL_Rect cursor_rect;
	cursor_rect.x = editor->cursor_display_x * font.width;
	cursor_rect.y = (editor->cursor_y - editor->row_offset) * font.height;
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
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	*cols = w / font.width;
	*rows = h / font.height;
}

void window_destroy()
{
	font_destroy(&font);

	SDL_DestroyTexture(font_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
