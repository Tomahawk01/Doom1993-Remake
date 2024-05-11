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

	wall_node** wall_node_ptr = &w_list;
	for (int i = 0; i < map.num_linedefs; i++)
	{
		linedef* ld = &map.linedefs[i];

		if (ld->flags & LINEDEF_FLAGS_TWO_SIDED)
		{
			// TODO:
		}
		else
		{
			wall_node* node = malloc(sizeof(wall_node));
			node->next = NULL;

			vec2 start = map.vertices[ld->start_index];
			vec2 end = map.vertices[ld->end_index];

			sector* sect = &map.sectors[map.sidedefs[ld->front_sidedef].sector_index];

			float floor = (float)sect->floor / SCALE;
			float ceiling = (float)sect->ceiling / SCALE;

			start.x /= SCALE;
			start.y /= SCALE;
			end.x /= SCALE;
			end.y /= SCALE;

			float x = end.x - start.x;
			float y = end.y - start.y;
			float length = sqrtf(x * x + y * y);
			float height = ceiling - floor;
			float angle = atan2f(y, x);

			mat4 translation = mat4_translate((vec3) { start.x, floor, start.y });
			mat4 scale = mat4_scale((vec3) { length, height, 1.0f });
			mat4 rotation = mat4_rotate((vec3) { 0.0f, 1.0f, 0.0f }, angle);
			node->model = mat4_mult(mat4_mult(scale, rotation), translation);
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
		vec3 color = get_random_color(node->sector);
		renderer_draw_mesh(&quad_mesh, node->model, (vec4) { color.x, color.y, color.z, 1.0f });
	}
}

vec3 get_random_color(const void* seed)
{
	srand((uintptr_t)seed);
	return (vec3){
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX
	};
}
