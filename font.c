#include "font.h"

#include <assert.h>
#include <stdlib.h>

/* Create a texture atlas containing all of the glyphs in a font. */
SDL_Texture *font_create_texture(SDL_Renderer *renderer, PSFFont *font)
{
	SDL_Texture *result;

	const int texture_width = font->num_glyphs * font->width;
	const int bits_per_glyph = 8;
	SDL_Surface *surface = SDL_CreateRGBSurface(0, texture_width, font->height, 32, 0, 0, 0, 0);

	for (int glyph_idx = 0; glyph_idx < font->num_glyphs; glyph_idx++) {
		for (int y = 0; y < font->height; y++) {
			for (int x = 0; x < font->width; x++) {
				/*
				 * TODO: Make it possible for the texture atlas to have more
				 * than one row.
				 */
				int xp = x + glyph_idx * bits_per_glyph;
				int yp = y;

				int bit_idx = 8 - 1 - x;
				char current_bit = font->glyph_data[glyph_idx * font->bytes_per_glyph + y] & (1 << bit_idx);
				((Uint32*)(surface->pixels))[xp + yp * surface->w] = current_bit ? 0xffffffff : 0;
			}
		}
	}

	result = SDL_CreateTextureFromSurface(renderer, surface);
	if (result == NULL) {
		fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
	}
	SDL_FreeSurface(surface);

	printf("Created font texture atlas of size %dx%d\n", surface->w, surface->h);
	return result;
}

PSFFont font_load(const char *filename)
{
	PSFFont font;

	FILE *fp = fopen(filename, "rb");

	fread(&font.magic, 4, 1, fp);
	assert(font.magic == PSF_MAGIC_NUMBER);

	fread(&font.version, 4, 1, fp);
	fread(&font.header_size, 4, 1, fp);
	fread(&font.flags, 4, 1, fp);
	fread(&font.num_glyphs, 4, 1, fp);
	fread(&font.bytes_per_glyph, 4, 1, fp);
	fread(&font.height, 4, 1, fp);
	fread(&font.width, 4, 1, fp);

	/* TODO: Implement unicode translation table. */

	size_t glyph_buffer_size = font.num_glyphs * font.bytes_per_glyph;
	font.glyph_data = malloc(glyph_buffer_size);
	fread(font.glyph_data, font.bytes_per_glyph, font.num_glyphs, fp);

	fclose(fp);

	printf("Loaded a font with %d glyphs of size %dx%d (%d bytes per glyph)\n", 
			font.num_glyphs, font.width, font.height, font.bytes_per_glyph);
	return font;
}

void font_destroy(PSFFont *font)
{
	free(font->glyph_data);
}