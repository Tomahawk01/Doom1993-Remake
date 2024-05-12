#pragma once
#include <stdint.h>

typedef struct patch
{
	uint16_t width, height;
	uint8_t* data;
} patch;
