#pragma once
#include "math/vector.h"
#include "glad/glad.h"

#include <stdint.h>
#include <stddef.h>

typedef struct wall_tex
{
	char name[8];
	uint16_t width, height;
	uint8_t* data;
} wall_tex;

GLuint generate_wall_texture_array(const wall_tex* textures, size_t num_textures, vec2* max_coords_array);
