#ifndef _FONT_H
#define _FONT_H

#include <SDL2/SDL.h>

#define PSF_MAGIC_NUMBER 0x864ab572

typedef struct {
} PSFFontHeader;

typedef struct {
	uint32_t magic;
	uint32_t version;
	uint32_t header_size;
	uint32_t flags;
	uint32_t num_glyphs;
	uint32_t bytes_per_glyph;
	uint32_t height;
	uint32_t width;
	uint8_t *glyph_data;
} PSFFont;

PSFFont font_load(const char *);
SDL_Texture *font_create_texture(SDL_Renderer *, PSFFont *);
void font_destroy(PSFFont *);

#endif
