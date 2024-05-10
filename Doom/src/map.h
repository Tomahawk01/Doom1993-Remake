#pragma once
#include "math/vector.h"

#include <stddef.h>

typedef struct map
{
	size_t num_vertices;
	vec2* vertices;

	vec2 min, max;
} map;
