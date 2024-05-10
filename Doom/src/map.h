#pragma once
#include "math/vector.h"

#include <stddef.h>
#include <stdint.h>

#define LINEDEF_FLAGS_TWO_SIDED 0x0004

typedef struct linedef
{
	uint16_t start_index;
	uint16_t end_index;
	uint16_t flags;
} linedef;

typedef struct map
{
	size_t num_vertices;
	vec2* vertices;
	vec2 min, max;

	size_t num_linedefs;
	linedef* linedefs;
} map;
