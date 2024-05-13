#pragma once
#include "glad/glad.h"

#include <stdint.h>

typedef struct wall_tex
{
	char name[8];
	uint16_t width, height;
	uint8_t* data;
} wall_tex;
