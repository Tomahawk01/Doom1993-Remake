#pragma once
#include "math/vector.h"
#include "math/matrix.h"
#include "mesh.h"

void renderer_init(int width, int height);
void renderer_clear();
void renderer_set_projection(mat4 projection);
void renderer_set_view(mat4 view);

vec2 renderer_get_size();

void renderer_draw_mesh(const mesh* mesh, mat4 transformation, vec4 color);
void renderer_draw_point(vec2 point, float size, vec4 color);
void renderer_draw_line(vec2 p0, vec2 p1, float width, vec4 color);
void renderer_draw_quad(vec2 center, vec2 size, float angle, vec4 color);
