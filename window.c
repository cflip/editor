#include "window.h"

#include <SDL2/SDL.h>

#include "buffer.h"
#include "editor.h"
#include "font.h"
#include "input.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *font_texture = NULL;

static PSFFont font;

void window_init()
{
	/* TODO: There should be a 'panic' method of sorts to be reused here. */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Failed to initalize SDL: %s\n", SDL_GetError());
		return;
	}

	window = SDL_CreateWindow("Bitmap font test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								640, 480, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		fprintf(stderr, "Failed to create a window: %s\n", SDL_GetError());
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) {
		fprintf(stderr, "Failed to create a renderer: %s\n", SDL_GetError());
		return;
	}

	font = font_load("terminus/ter-u12n.psf");
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
		int keycode = e.key.keysym.sym;
		switch (e.key.keysym.sym) {
		case SDLK_BACKSPACE:
			keycode = BACKSPACE;
			break;
		case SDLK_LEFT:
			keycode = ARROW_LEFT;
			break;
		case SDLK_RIGHT:
			keycode = ARROW_RIGHT;
			break;
		case SDLK_UP:
			keycode = ARROW_UP;
			break;
		case SDLK_DOWN:
			keycode = ARROW_DOWN;
			break;
		case SDLK_DELETE:
			keycode = DELETE_KEY;
			break;
		case SDLK_HOME:
			keycode = HOME_KEY;
			break;
		case SDLK_END:
			keycode = END_KEY;
			break;
		case SDLK_PAGEUP:
			keycode = PAGE_UP;
			break;
		case SDLK_PAGEDOWN:
			keycode = PAGE_DOWN;
			break;
		}
		if (e.key.keysym.mod & KMOD_CTRL) {
			keycode = CTRL_KEY(keycode);
		}
		editor_process_keypress(editor, keycode);
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

		if (letter == '\n') {
			dstrect.x = 0;
			dstrect.y += font.height;
		} else {
			dstrect.x += font.width;
		}
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

	SDL_RenderPresent(renderer);
	SDL_UpdateWindowSurface(window);
}

void window_destroy()
{
	font_destroy(&font);

	SDL_DestroyTexture(font_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
