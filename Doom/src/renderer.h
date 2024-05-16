#pragma once
#include "math/vector.h"
#include "math/matrix.h"
#include "mesh.h"

void renderer_init(int width, int height);
void renderer_clear();

void renderer_set_palette_texture(GLuint palette_texture);
void renderer_set_palette_index(int index);
void renderer_set_wall_texture(GLuint texture);
void renderer_set_flat_texture(GLuint texture);
void renderer_set_projection(mat4 projection);
void renderer_set_view(mat4 view);

vec2 renderer_get_size();

void renderer_draw_mesh(const mesh* mesh, mat4 transformation);
