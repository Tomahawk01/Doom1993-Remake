#pragma once
#include "math/vector.h"

#include <stddef.h>
#include <stdint.h>

#define LINEDEF_FLAGS_TWO_SIDED 0x0004

typedef struct sector
{
	int16_t floor;
	int16_t ceiling;

	int16_t light_level;
} sector;

typedef struct sidedef
{
	uint16_t sector_index;
} sidedef;

typedef struct linedef
{
	uint16_t start_index;
	uint16_t end_index;
	uint16_t flags;
	uint16_t front_sidedef;
	uint16_t back_sidedef;
} linedef;

typedef struct map
{
	size_t num_vertices;
	vec2* vertices;
	vec2 min, max;

	size_t num_linedefs;
	linedef* linedefs;

	size_t num_sidedefs;
	sidedef* sidedefs;

	size_t num_sectors;
	sector* sectors;
} map;
