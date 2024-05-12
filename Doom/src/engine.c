#include "engine.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "map.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOV (M_PI / 2.0f)	// 90 degrees
#define PLAYER_SPEED (5.0f)
#define MOUSE_SENSITIVITY (0.002f) // in radians

#define SCALE (100.0f)

typedef struct wall_node
{
	mat4 model;
	const sector* sector;
	struct wall_node* next;
} wall_node, wall_list;

static vec3 get_random_color(const void* seed);
static mat4 model_from_vertices(vec3 p0, vec3 p1, vec3 p2, vec3 p3);

static camera cam;
static vec2 last_mouse_pos;
static mesh quad_mesh;

static wall_list* w_list;

void engine_init(wad* wad, const char* mapname)
{
	cam = (camera){
		.position = {0.0f, 0.0f, 3.0f},
		.yaw = -M_PI_2,
		.pitch = 0.0f
	};

	vec2 size = renderer_get_size();
	mat4 projection = mat4_perspective(FOV, size.x / size.y, 0.1f, 1000.0f);
	renderer_set_projection(projection);

	vertex vertices[] = {
		{.position = { 1.0f, 1.0f, 0.0f}},		// top-right
		{.position = { 0.0f, 1.0f, 0.0f}},		// bottom-right
		{.position = { 0.0f, 0.0f, 0.0f}},		// bottom-left
		{.position = { 1.0f, 0.0f, 0.0f}}		// top-left
	};

	uint32_t indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	mesh_create(&quad_mesh, 4, vertices, 6, indices);

	map map;
	if (wad_read_map(mapname, &map, wad) != 0)
	{
		fprintf(stderr, "Failed to read map '%s' from WAD file\n", mapname);
		return;
	}

	char* gl_mapname = malloc(strlen(mapname) + 4);
	gl_mapname[0] = 'G';
	gl_mapname[1] = 'L';
	gl_mapname[2] = '_';
	gl_mapname[3] = 0;
	strcat(gl_mapname, mapname);

	gl_map gl_map;
	if (wad_read_gl_map(gl_mapname, &gl_map, wad) != 0)
	{
		fprintf(stderr, "Failed to read GL info for map '%s' from WAD file\n", mapname);
		return;
	}

	wall_node** wall_node_ptr = &w_list;
	for (int i = 0; i < map.num_linedefs; i++)
	{
		linedef* ld = &map.linedefs[i];

		if (ld->flags & LINEDEF_FLAGS_TWO_SIDED)
		{
			// Floor
			wall_node* floor_node = malloc(sizeof(wall_node));
			floor_node->next = NULL;

			vec2 start = map.vertices[ld->start_index];
			vec2 end = map.vertices[ld->end_index];

			sidedef* front_side = &map.sidedefs[ld->front_sidedef];
			sector* front_sect = &map.sectors[front_side->sector_index];
			sidedef* back_side = &map.sidedefs[ld->back_sidedef];
			sector* back_sect = &map.sectors[back_side->sector_index];

			vec3 floor0 = { start.x, front_sect->floor, start.y };
			vec3 floor1 = { end.x, front_sect->floor, end.y };
			vec3 floor2 = { end.x, back_sect->floor, end.y };
			vec3 floor3 = { start.x, back_sect->floor, start.y };

			floor_node->model = model_from_vertices(floor0, floor1, floor2, floor3);
			floor_node->sector = front_sect;

			*wall_node_ptr = floor_node;
			wall_node_ptr = &floor_node->next;

			// Ceiling
			wall_node* ceil_node = malloc(sizeof(wall_node));
			ceil_node->next = NULL;

			vec3 ceil0 = { start.x, front_sect->ceiling, start.y };
			vec3 ceil1 = { end.x, front_sect->ceiling, end.y };
			vec3 ceil2 = { end.x, back_sect->ceiling, end.y };
			vec3 ceil3 = { start.x, back_sect->ceiling, start.y };

			ceil_node->model = model_from_vertices(ceil0, ceil1, ceil2, ceil3);
			ceil_node->sector = front_sect;

			*wall_node_ptr = ceil_node;
			wall_node_ptr = &ceil_node->next;
		}
		else
		{
			wall_node* node = malloc(sizeof(wall_node));
			node->next = NULL;

			vec2 start = map.vertices[ld->start_index];
			vec2 end = map.vertices[ld->end_index];

			sidedef* side = &map.sidedefs[ld->front_sidedef];
			sector* sect = &map.sectors[side->sector_index];

			vec3 p0 = { start.x, sect->floor, start.y };
			vec3 p1 = { end.x, sect->floor, end.y };
			vec3 p2 = { end.x, sect->ceiling, end.y };
			vec3 p3 = { start.x, sect->ceiling, start.y };

			node->model = model_from_vertices(p0, p1, p2, p3);
			node->sector = sect;

			*wall_node_ptr = node;
			wall_node_ptr = &node->next;
		}
	}
}

void engine_update(float dt)
{
	camera_update_direction_vertors(&cam);

	float speed = (is_button_pressed(KEY_LSHIFT) ? PLAYER_SPEED * 1.7f : PLAYER_SPEED) * dt;

	if (is_button_pressed(KEY_W))
		cam.position = vec3_add(cam.position, vec3_scale(cam.forward, speed));
	if (is_button_pressed(KEY_S))
		cam.position = vec3_add(cam.position, vec3_scale(cam.forward, -speed));
	if (is_button_pressed(KEY_A))
		cam.position = vec3_add(cam.position, vec3_scale(cam.right, -speed));
	if (is_button_pressed(KEY_D))
		cam.position = vec3_add(cam.position, vec3_scale(cam.right, speed));

	if (is_button_pressed(MOUSE_RIGHT))
	{
		if (!is_mouse_captured())
		{
			last_mouse_pos = get_mouse_position();
			set_mouse_captured(1);
		}

		vec2 curr_mouse_pos = get_mouse_position();
		float dx = curr_mouse_pos.x - last_mouse_pos.x;
		float dy = last_mouse_pos.y - curr_mouse_pos.y;
		last_mouse_pos = curr_mouse_pos;

		cam.yaw += dx * MOUSE_SENSITIVITY;
		cam.pitch += dy * MOUSE_SENSITIVITY;
		// Clamp the camera vertically
		cam.pitch = max(-M_PI_2 + 0.05, min(M_PI_2 - 0.05, cam.pitch));
	}
	else if (is_mouse_captured())
	{
		set_mouse_captured(0);
	}
}

void engine_render()
{
	mat4 view = mat4_look_at(cam.position, vec3_add(cam.position, cam.forward), cam.up);
	renderer_set_view(view);

	for (wall_node* node = w_list; node != NULL; node = node->next)
	{
		vec3 color = vec3_scale(get_random_color(node->sector), node->sector->light_level / 255.0f);
		renderer_draw_mesh(&quad_mesh, node->model, (vec4) { color.x, color.y, color.z, 1.0f });
	}
}

vec3 get_random_color(const void* seed)
{
	srand((uintptr_t)seed);
	return vec3_normalize((vec3){
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX
	});
}

mat4 model_from_vertices(vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
	p0 = vec3_scale(p0, 1.0f / SCALE);
	p1 = vec3_scale(p1, 1.0f / SCALE);
	p2 = vec3_scale(p2, 1.0f / SCALE);
	p3 = vec3_scale(p3, 1.0f / SCALE);

	float x = p1.x - p0.x;
	float y = p1.z - p0.z;
	float length = sqrtf(x * x + y * y);
	float height = p3.y - p0.y;
	float angle = atan2f(y, x);

	mat4 translation = mat4_translate(p0);
	mat4 scale = mat4_scale((vec3) { length, height, 1.0f });
	mat4 rotation = mat4_rotate((vec3) { 0.0f, 1.0f, 0.0f }, angle);
	return mat4_mult(mat4_mult(scale, rotation), translation);
}
