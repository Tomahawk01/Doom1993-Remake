#pragma once

#include <stdint.h>

typedef struct lump
{
	char* name;
	uint8_t* data;
	uint32_t offset;
	uint32_t size;
} lump;

typedef struct wad
{
	char* id;
	uint32_t num_lumps;
	uint32_t directory_offset;

	lump* lumps;
} wad;

int wad_load_from_file(const char* filename, wad* wad);
void wad_free(wad* wad);
