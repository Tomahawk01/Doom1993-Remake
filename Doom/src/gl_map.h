#pragma once
#include "math/vector.h"

#include <stddef.h>
#include <stdint.h>

#define VERT_IS_GL (1 << 15)

typedef struct gl_subsector
{
	uint16_t num_segs;
	uint16_t first_seg;
} gl_subsector;

typedef struct gl_segment
{
	uint16_t start_vertex;
	uint16_t end_vertex;
	uint16_t linedef;
	uint16_t side;
} gl_segment;

typedef struct gl_node
{
	uint16_t front_child_id;
	uint16_t back_child_id;
	vec2 partition;
	vec2 delta_partition;
	int16_t front_bbox[4];
	int16_t back_bbox[4];
} gl_node;

typedef struct gl_map
{
	size_t num_vertices;
	vec2* vertices;
	vec2 min;
	vec2 max;

	size_t num_segments;
	gl_segment* segments;

	size_t num_subsectors;
	gl_subsector* subsectors;

	size_t num_nodes;
	gl_node* nodes;
} gl_map;
