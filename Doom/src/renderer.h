#pragma once
#include "math/vector.h"

void renderer_init(int width, int height);
void renderer_clear();

void renderer_draw_point(vec2 point, float size, vec4 color);
void renderer_draw_line(vec2 p0, vec2 p1, float width, vec4 color);
void renderer_draw_quad(vec2 center, vec2 size, float angle, vec4 color);
