#include "window.h"

#include <SDL2/SDL.h>
#include "font.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *font_texture = NULL;

static BDFFontInfo font;

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

	font = font_load("ter-u12n.bdf");
	font_texture = font_create_texture(renderer, &font);

	SDL_ShowWindow(window);
}

int window_handle_event()
{
	static SDL_Event e;
	SDL_WaitEvent(&e);
	switch (e.type) {
	case SDL_QUIT:
		return 0;
	}
	return 1;
}

void window_redraw(struct editor_state *editor)
{
	const char *testmsg = "Hello, welcome to the bitmap font test!";
	SDL_RenderClear(renderer);
	SDL_Rect dstrect = { 0, 0, 0, 0 };
	for (int i = 0; i < strlen(testmsg); i++) {
		const char letter = testmsg[i];
		const int char_index = font.char_index_for_code_point[letter];
		BDFFontChar *glyph = &font.chars[char_index];

		dstrect.w = glyph->bounds.w;
		dstrect.h = glyph->bounds.h;
		SDL_RenderCopy(renderer, font_texture, &glyph->bounds, &dstrect);

		dstrect.x += glyph->next_glyph_offset.x;
		dstrect.y += glyph->next_glyph_offset.y;
	}
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
