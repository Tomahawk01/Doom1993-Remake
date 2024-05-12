#pragma once
#include "glad/glad.h"

#include <stdint.h>

#define NUM_COLORS 256

typedef struct palette
{
	uint8_t colors[NUM_COLORS * 3];
} palette;

GLuint palette_generate_texture(const palette* pal);
