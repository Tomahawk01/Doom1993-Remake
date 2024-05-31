#pragma once
#include "mesh.h"
#include <stddef.h>

#define TEX_ANIM_TIME (8.0f / 35.0f)

typedef struct tex_anim
{
    mesh* mesh;
    size_t vertex_index_start;
    size_t vertex_index_end;
    int min_tex;
    int max_tex;
    float time;

    struct tex_anim* next;
} flat_anim_t;

void update_animation(float dt);

void add_tex_anim(mesh* mesh, size_t vertex_index_start, size_t vertex_index_end, int min_tex, int max_tex);
