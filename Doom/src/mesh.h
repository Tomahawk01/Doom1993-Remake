#pragma once
#include "math/vector.h"
#include "glad/glad.h"

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
} vertex;

void mesh_create(mesh* mesh, size_t num_vertices, vertex* vertices, size_t num_indices, uint32_t* indices);