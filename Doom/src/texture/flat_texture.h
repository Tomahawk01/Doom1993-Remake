#pragma once
#include "glad/glad.h"

#include <stddef.h>
#include <stdint.h>

#define FLAT_TEXTURE_SIZE 64

typedef struct flat_tex
{
	uint8_t data[FLAT_TEXTURE_SIZE * FLAT_TEXTURE_SIZE];
} flat_tex;

GLuint generate_flat_texture_array(const flat_tex* flats, size_t num_flats);
