#include "font.h"

/* Create a texture containing all of the glyphs in a specified BDF file. */
SDL_Texture *font_create_texture(SDL_Renderer *renderer, BDFFontInfo *font)
{
	SDL_Texture *result;
	SDL_Surface *surface = SDL_CreateRGBSurface(0, 8 * font->num_chars, font->bounds.h, 32, 0, 0, 0, 0);

	printf("Made surface of size %dx%d\n", surface->w, surface->h);
	SDL_LockSurface(surface);
	for (int i = 0; i < font->num_chars; i++) {
		BDFFontChar chair = font->chars[i];
		for (int y = 0; y < font->bounds.h; y++) {
			int bits_for_row = chair.bitmap[y];
			for (int x = 0; x < 8; x++) {
				int bit_index = 7 - x;
				int current_bit = bits_for_row & (1 << bit_index);
				int xp = x + i * 8;
				int yp = y;
				((Uint32*)(surface->pixels))[xp + yp * surface->w] = current_bit ? 0xffffffff : 0;
			}
		}
	}
	SDL_UnlockSurface(surface);

	result = SDL_CreateTextureFromSurface(renderer, surface);
	if (result == NULL) {
		fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
	}
	SDL_FreeSurface(surface);
	return result;
}

/* 
 * Loads a BDF font file into a data structure.
 * TODO: This is extremely atrocious!
 */
BDFFontInfo font_load(const char *filename)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t bytes_read;

	BDFFontInfo result;
	result.bounds.x = 0;
	result.bounds.y = 0;
	result.bounds.w = 0;
	result.bounds.h = 0;
	result.chars = NULL;
	result.num_chars = 0;
	for (int i = 0; i < 128; i++) {
		result.char_index_for_code_point[i] = 0;
	}

	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open file from %s\n", filename);
		return;
	}

	int is_reading_bitmap = 0;
	int char_counter = -1;
	while ((bytes_read = getline(&line, &len, fp)) != -1) {
		if (strncmp(line, "FONTBOUNDINGBOX", 15) == 0) {
			sscanf(line, "FONTBOUNDINGBOX %d %d %d %d\n", &result.bounds.w,
					                                      &result.bounds.h,
					                                      &result.bounds.x,
					                                      &result.bounds.y);

		}

		if (strncmp(line, "CHARS", 5) == 0) {
			sscanf(line, "CHARS %d\n", &result.num_chars);
			result.chars = malloc(result.num_chars * sizeof(BDFFontChar));
		}

		if (strncmp(line, "STARTCHAR", 9) == 0) {
			char_counter++;
			result.chars[char_counter].code_point = 0;
			result.chars[char_counter].next_glyph_offset.x = 0;
			result.chars[char_counter].next_glyph_offset.y = 0;
			result.chars[char_counter].bounds.x = 0;
			result.chars[char_counter].bounds.y = 0;
			result.chars[char_counter].bounds.w = 0;
			result.chars[char_counter].bounds.h = 0;
			result.chars[char_counter].bitmap = NULL;
			result.chars[char_counter].bitmap_size = 0;
		}

		if (strncmp(line, "ENCODING", 8) == 0) {
			int code_point = 0;
			sscanf(line, "ENCODING %d\n", &code_point);
			if (code_point < 256)
				result.char_index_for_code_point[code_point] = char_counter;
			result.chars[char_counter].code_point = code_point;
		}

		if (strncmp(line, "DWIDTH", 6) == 0) {
			sscanf(line, "DWIDTH %d %d\n", &result.chars[char_counter].next_glyph_offset.x,
				                          &result.chars[char_counter].next_glyph_offset.y);
		}

		if (strncmp(line, "BBX", 3) == 0) {
			sscanf(line, "BBX %d %d %d %d\n", &result.chars[char_counter].bounds.w,
					                          &result.chars[char_counter].bounds.h,
					                          &result.chars[char_counter].bounds.x,
					                          &result.chars[char_counter].bounds.y);
			result.chars[char_counter].bounds.x += char_counter * 8;
			result.chars[char_counter].bounds.y = -result.chars[char_counter].bounds.y;
		}

		if (strncmp(line, "BITMAP", 6) == 0) {
			is_reading_bitmap = 1;
		}

		if (is_reading_bitmap) {
			if (strncmp(line, "ENDCHAR", 7) == 0) {
				is_reading_bitmap = 0;
				continue;
			}

			BDFFontChar *chair = &result.chars[char_counter];
			chair->bitmap = realloc(chair->bitmap, chair->bitmap_size++);
			chair->bitmap[chair->bitmap_size - 1] = strtol(line, NULL, 16);
		}
	}

	fclose(fp);
	if (line) { free(line); }

	return result;
}

void font_destroy(BDFFontInfo *font)
{
	for (int i = 0; i < font->num_chars; i++)
		free(font->chars[i].bitmap);
	free(font->chars);
}
