#ifndef _FONT_H
#define _FONT_H

#include <SDL2/SDL.h>

typedef struct {
	int code_point;
	SDL_Point next_glyph_offset;
	SDL_Rect bounds;
	char *bitmap;
	int bitmap_size;
} BDFFontChar;

typedef struct {
	SDL_Rect bounds;
	BDFFontChar *chars;
	int num_chars;
	int char_index_for_code_point[128];
} BDFFontInfo;

BDFFontInfo font_load(const char *);
SDL_Texture *font_create_texture(SDL_Renderer *, BDFFontInfo *);
void font_destroy(BDFFontInfo *);

#endif
