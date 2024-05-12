#pragma once
#include "map.h"
#include "gl_map.h"
#include "palette.h"
#include "flat_texture.h"
#include "patch.h"

#include <stdint.h>

typedef struct lump
{
	char* name;
	uint8_t* data;
	uint32_t size;
} lump;

typedef struct wad
{
	char* id;
	uint32_t num_lumps;

	lump* lumps;
} wad;

int wad_load_from_file(const char* filename, wad* wad);
void wad_free(wad* wad);

int wad_find_lump(const char* lumpname, const wad* wad);
int wad_read_map(const char* mapname, map* map, const wad* wad);
int wad_read_gl_map(const char* gl_mapname, gl_map* map, const wad* wad);

int wad_read_patch(patch* patch, const char* patch_name, const wad* wad);
int wad_read_playpal(palette* pal, const wad* wad);
flat_tex* wad_read_flats(size_t* num, const wad* wad);
patch* wad_read_patches(size_t* num, const wad* wad);
