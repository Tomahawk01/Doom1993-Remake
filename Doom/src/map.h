#pragma once
#include "math/vector.h"

#include <stddef.h>
#include <stdint.h>

#define LINEDEF_FLAGS_TWO_SIDED 0x0004
#define LINEDEF_FLAGS_UPPER_UNPEGGED 0x0008
#define LINEDEF_FLAGS_LOWER_UNPEGGED 0x0010

enum thing_types
{
	THING_P1_START = 1
};

typedef struct thing_info
{
	uint16_t type;
	int height;
} thing_info;

extern int map_num_thing_infos;
extern thing_info map_thing_info[];

typedef struct sector
{
	int16_t floor;
	int16_t ceiling;

	int16_t light_level;

	int floor_tex;
	int ceiling_tex;
} sector;

typedef struct sidedef
{
	int16_t x_off, y_off;
	int upper, lower, middle;
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

typedef struct thing
{
	uint16_t type;
	vec2 position;
	float angle;
} thing;

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

	size_t num_things;
	thing* things;
} map;
