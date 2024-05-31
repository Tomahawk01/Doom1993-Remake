#include "meshgen.h"
#include "engine.h"
#include "camera.h"
#include "gl_map.h"
#include "input.h"
#include "map.h"
#include "mesh.h"
#include "palette.h"
#include "renderer.h"
#include "utils.h"
#include "wad_loader.h"
#include "texture/flat_texture.h"
#include "texture/wall_texture.h"
#include "engine/state.h"
#include "engine/utilities.h"
#include "engine/anim.h"
#include "math/matrix.h"
#include "math/vector.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOV (M_PI / 2.0f)	// 90 degrees
#define PLAYER_SPEED (500.0f)
#define MOUSE_SENSITIVITY (0.002f) // in radians

static void render_node(draw_node* node);

size_t num_flats, num_wall_textures, num_palettes;
wall_tex_info* wall_textures_info;
vec2* wall_max_coords;

map m;
gl_map gl_m;
float player_height;
float max_sector_height;
int sky_flat;

draw_node* root_draw_node;
stencil_list stencil_ls;
mesh quad_mesh;

size_t num_tex_anim_defs;
tex_anim_def tex_anim_defs[] = {
	{"NUKAGE3", "NUKAGE1"},
	{"FWATER4", "FWATER1"},
	{"SWATER4", "SWATER1"},
	{"LAVA4",	"LAVA1"},
	{"BLOOD3",	"BLOOD1"},
};

static camera cam;
static vec2 last_mouse;

void engine_init(wad* wad, const char* mapname)
{
	vec2 size = renderer_get_size();
	mat4 projection = mat4_perspective(FOV, size.x / size.y, 0.1f, 10000.0f);
	renderer_set_projection(projection);

	char* gl_mapname = malloc(strlen(mapname) + 4);
	gl_mapname[0] = 'G';
	gl_mapname[1] = 'L';
	gl_mapname[2] = '_';
	gl_mapname[3] = 0;
	strcat(gl_mapname, mapname);

	if (wad_read_gl_map(gl_mapname, &gl_m, wad) != 0)
	{
		fprintf(stderr, "Failed to read GL info for map '%s' from WAD file\n", mapname);
		return;
	}

	palette* palettes = wad_read_playpal(&num_palettes, wad);
	GLuint palette_texture = palettes_generate_texture(palettes, num_palettes);

	sky_flat = wad_find_lump("F_SKY1", wad) - wad_find_lump("F_START", wad) - 1;

	num_tex_anim_defs = sizeof tex_anim_defs / sizeof tex_anim_defs[0];
	for (int i = 0; i < num_tex_anim_defs; i++)
		tex_anim_defs[i].start = tex_anim_defs[i].end = -1;

	flat_tex* flats = wad_read_flats(&num_flats, wad);
	GLuint flat_texture_array = generate_flat_texture_array(flats, num_flats);
	for (int i = 0; i < num_flats; i++)
	{
		for (int j = 0; j < num_tex_anim_defs; j++)
		{
			if (strcmp_nocase(flats[i].name, tex_anim_defs[j].start_name) == 0)
				tex_anim_defs[j].start = i;

			if (strcmp_nocase(flats[i].name, tex_anim_defs[j].end_name) == 0)
				tex_anim_defs[j].end = i;
		}
	}
	free(flats);

	wall_tex* textures = wad_read_textures(&num_wall_textures, "TEXTURE1", wad);
	wall_textures_info = malloc(sizeof(wall_tex_info) * num_wall_textures);
	wall_max_coords = malloc(sizeof(vec2) * num_wall_textures);
	for (int i = 0; i < num_wall_textures; i++)
	{
		if (strcmp_nocase(textures[i].name, "SKY1") == 0)
			renderer_set_sky_texture(generate_texture_cubemap(&textures[i]));

		wall_textures_info[i] = (wall_tex_info){ textures[i].width, textures[i].height };
	}

	GLuint wall_texture_array = generate_wall_texture_array(textures, num_wall_textures, wall_max_coords);
	wad_free_wall_textures(textures, num_wall_textures);

	if (wad_read_map(mapname, &m, wad, textures, num_wall_textures) != 0)
	{
		fprintf(stderr, "Failed to read map '%s' from WAD file\n", mapname);
		return;
	}

	for (int i = 0; i < m.num_things; i++)
	{
		thing* thing = &m.things[i];

		if (thing->type == THING_P1_START)
		{
			thing_info* info = NULL;

			for (int i = 0; i < map_num_thing_infos; i++)
			{
				if (thing->type == map_thing_info[i].type)
				{
					info = &map_thing_info[i];
					break;
				}
			}

			if (info == NULL)
				continue;

			player_height = info->height;

			cam = (camera){
				.position = {thing->position.x, player_height, thing->position.y},
				.yaw = thing->angle,
				.pitch = 0.0f
			};
		}
	}

	stencil_ls = (stencil_list){ NULL, NULL };
	generate_meshes();

	renderer_set_flat_texture(flat_texture_array);
	renderer_set_wall_texture(wall_texture_array);
	renderer_set_palette_texture(palette_texture);

	vec3 stencil_quad_vertices[] = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{1.0f, 1.0f, 0.0f},
		{1.0f, 0.0f, 0.0f}
	};

	uint32_t stencil_quad_indices[] = { 0, 2, 1, 0, 3, 2 };

	mesh_create(&quad_mesh, VERTEX_LAYOUT_PLAIN, 4, stencil_quad_vertices, 6, stencil_quad_indices, false);
}

static int palette_index = 0;
void engine_update(float dt)
{
	if (is_button_just_pressed(KEY_MINUS))
		palette_index--;
	if (is_button_just_pressed(KEY_EQUAL))
		palette_index++;

	palette_index = min(max(palette_index, 0), num_palettes - 1);

	camera_update_direction_vectors(&cam);

	vec2 position = { cam.position.x, cam.position.z };
	sector* sector = map_get_sector(position);
	if (sector)
		cam.position.y = sector->floor + player_height;

	float speed = (is_button_pressed(KEY_LSHIFT) ? PLAYER_SPEED * 1.7f : PLAYER_SPEED) * dt;

	vec3 forward = cam.forward;
	vec3 right = cam.right;
	forward.y = right.y = 0.0f;
	forward = vec3_normalize(forward);
	right = vec3_normalize(right);

	if (is_button_pressed(KEY_W) || is_button_pressed(KEY_UP))
		cam.position = vec3_add(cam.position, vec3_scale(forward, speed));
	if (is_button_pressed(KEY_S) || is_button_pressed(KEY_DOWN))
		cam.position = vec3_add(cam.position, vec3_scale(forward, -speed));
	if (is_button_pressed(KEY_A))
		cam.position = vec3_add(cam.position, vec3_scale(right, -speed));
	if (is_button_pressed(KEY_D))
		cam.position = vec3_add(cam.position, vec3_scale(right, speed));

	float turn_speed = 4.0f * dt;
	if (is_button_pressed(KEY_RIGHT))
		cam.yaw -= turn_speed;
	if (is_button_pressed(KEY_LEFT))
		cam.yaw += turn_speed;

	if (is_button_pressed(KEY_ESCAPE))
		set_mouse_captured(0);
	if (is_button_pressed(MOUSE_RIGHT))
		set_mouse_captured(1);

	if (is_button_pressed(KEY_EQUAL))
		cam.pitch = 0.0f;

	static bool is_first = true;
	if (is_mouse_captured())
	{
		if (is_first)
		{
			last_mouse = get_mouse_position();
			is_first = false;
		}

		vec2 curr_mouse_pos = get_mouse_position();
		float dx = curr_mouse_pos.x - last_mouse.x;
		float dy = last_mouse.y - curr_mouse_pos.y;
		last_mouse = curr_mouse_pos;

		cam.yaw -= dx * MOUSE_SENSITIVITY;
		cam.pitch += dy * MOUSE_SENSITIVITY;
		// Clamp the camera vertically
		cam.pitch = max(-M_PI_2 + 0.05, min(M_PI_2 - 0.05, cam.pitch));
	}
	else
	{
		is_first = true;
	}

	update_animation(dt);
}

void engine_render()
{
	mat4 view = mat4_look_at(cam.position, vec3_add(cam.position, cam.forward), cam.up);
	renderer_set_view(view);

	renderer_set_palette_index(palette_index);

	glStencilMask(0x00);
	render_node(root_draw_node);

	glStencilMask(0xff);
	for (stencil_node* node = stencil_ls.head; node != NULL; node = node->next)
	{
		renderer_draw_mesh(&quad_mesh, SHADER_PLAIN, node->transformation);
	}

	renderer_draw_sky();
}

void render_node(draw_node* node)
{
	if (node->mesh)
		renderer_draw_mesh(node->mesh, SHADER_DEFAULT, mat4_identity());
	if (node->front)
		render_node(node->front);
	if (node->back)
		render_node(node->back);
}
