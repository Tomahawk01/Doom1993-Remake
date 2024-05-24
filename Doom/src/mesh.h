#pragma once
#include "math/vector.h"
#include "glad/glad.h"
#include "darray.h"

#include <stddef.h>
#include <stdint.h>

typedef struct mesh
{
	GLuint vao, vbo, ebo;
	size_t num_indices;
} mesh;

typedef struct vertex
{
	vec3 position;
	vec2 tex_coords;
	int texture_index;
	int texture_type;
	float light;
	vec2 max_coords;
} vertex;

typedef enum vertex_layout
{
	VERTEX_LAYOUT_PLAIN,
	VERTEX_LAYOUT_FULL
} vertex_layout;

void mesh_create(mesh* mesh, vertex_layout vertex_layout, size_t num_vertices, const void* vertices, size_t num_indices, const uint32_t* indices);

typedef darray(vertex) vertexarray;
typedef darray(uint32_t) indexarray;
