#include "mesh.h"
#include "math/vector.h"

void mesh_create(mesh* mesh, vertex_layout vertex_layout, size_t num_vertices, const void* vertices, size_t num_indices, const uint32_t* indices)
{
	mesh->num_indices = num_indices;

	glGenVertexArrays(1, &mesh->vao);
	glGenBuffers(1, &mesh->vbo);
	glGenBuffers(1, &mesh->ebo);

	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	
	switch (vertex_layout)
	{
	case VERTEX_LAYOUT_PLAIN:
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * num_vertices, vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
		glEnableVertexAttribArray(0);
		break;
	case VERTEX_LAYOUT_FULL:
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * num_vertices, vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tex_coords));
		glEnableVertexAttribArray(1);
		
		glVertexAttribIPointer(2, 1, GL_INT, sizeof(vertex), (void*)offsetof(vertex, texture_index));
		glEnableVertexAttribArray(2);

		glVertexAttribIPointer(3, 1, GL_INT, sizeof(vertex), (void*)offsetof(vertex, texture_type));
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, light));
		glEnableVertexAttribArray(4);

		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, max_coords));
		glEnableVertexAttribArray(5);
		break;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * num_indices, indices, GL_STATIC_DRAW);
}
