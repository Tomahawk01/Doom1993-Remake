#include "engine.h"
#include "renderer.h"
#include "camera.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#define FOV (M_PI / 2.0f)	// 90 degrees

static camera cam;
static mesh quad_mesh;

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

	map map;
	if (wad_read_map(mapname, &map, wad) != 0)
	{
		fprintf(stderr, "Failed to read map '%s' from WAD file\n", mapname);
		return;
	}

	vertex vertices[] = {
		{.position = { 0.5f, 0.5f, 0.0f}},		// top-right
		{.position = { 0.5f,-0.5f, 0.0f}},		// bottom-right
		{.position = {-0.5f,-0.5f, 0.0f}},		// bottom-left
		{.position = {-0.5f, 0.5f, 0.0f}}		// top-left
	};

	uint32_t indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	mesh_create(&quad_mesh, 4, vertices, 6, indices);
}

void engine_update(float dt)
{
	camera_update_direction_vertors(&cam);
}

void engine_render()
{
	mat4 view = mat4_look_at(cam.position, vec3_add(cam.position, cam.forward), cam.up);
	renderer_set_view(view);

	renderer_draw_mesh(&quad_mesh, mat4_identity(), (vec4) { 1.0f, 1.0f, 1.0f, 1.0f });
}
