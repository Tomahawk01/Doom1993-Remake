#pragma once

#include "math/matrix.h"
#include "gl_map.h"
#include "map.h"
#include "mesh.h"

typedef struct draw_node
{
	mesh* mesh;
	struct draw_node* front;
	struct draw_node* back;
} draw_node;

typedef struct stencil_node
{
	mat4 transformation;
	struct stencil_node* next;
} stencil_node;

typedef struct stencil_quad_list
{
	stencil_node* head;
	stencil_node* tail;
} stencil_list;

typedef struct wall_tex_info
{
	int width;
	int height;
} wall_tex_info;

typedef struct tex_anim_def
{
	const char* end_name;
	const char* start_name;
	int start;
	int end;
} tex_anim_def;

extern size_t num_flats;
extern size_t num_wall_textures;
extern size_t num_palettes;
extern wall_tex_info* wall_textures_info;
extern vec2* wall_max_coords;

extern map m;
extern gl_map gl_m;
extern float player_height;
extern float max_sector_height;
extern int sky_flat;

extern draw_node* root_draw_node;
extern stencil_list stencil_ls;

extern tex_anim_def tex_anim_defs[];
extern size_t num_tex_anim_defs;
