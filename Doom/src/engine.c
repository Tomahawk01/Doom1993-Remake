#include "engine.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define FOV (M_PI / 2.0f)	// 90 degrees
#define PLAYER_SPEED (5.0f)
#define MOUSE_SENSITIVITY (0.002f) // in radians

static camera cam;
static vec2 last_mouse_pos;
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

	float speed = (is_button_pressed(KEY_LSHIFT) ? PLAYER_SPEED * 1.7f : PLAYER_SPEED) * dt;

	if (is_button_pressed(KEY_W))
		cam.position = vec3_add(cam.position, vec3_scale(cam.forward, speed));
	if (is_button_pressed(KEY_S))
		cam.position = vec3_add(cam.position, vec3_scale(cam.forward, -speed));
	if (is_button_pressed(KEY_A))
		cam.position = vec3_add(cam.position, vec3_scale(cam.right, speed));
	if (is_button_pressed(KEY_D))
		cam.position = vec3_add(cam.position, vec3_scale(cam.right, -speed));

	if (is_button_pressed(MOUSE_RIGHT))
	{
		if (!is_mouse_captured())
		{
			last_mouse_pos = get_mouse_position();
			set_mouse_captured(1);
		}

		vec2 curr_mouse_pos = get_mouse_position();
		float dx = last_mouse_pos.x - curr_mouse_pos.x;
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

	renderer_draw_mesh(&quad_mesh, mat4_identity(), (vec4) { 1.0f, 1.0f, 1.0f, 1.0f });
}
