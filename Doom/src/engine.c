#include "engine.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "map.h"
#include "palette.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOV (M_PI / 2.0f)	// 90 degrees
#define PLAYER_SPEED (5.0f)
#define MOUSE_SENSITIVITY (0.002f) // in radians

#define SCALE (100.0f)

typedef struct draw_node
{
	mesh mesh;
	const sector* sector;
	struct draw_node* next;
} draw_node, draw_list;

static void generate_meshes(const map* map, const gl_map* gl_map);

static palette pal;
static size_t num_flats;
static size_t num_wall_textures;
static GLuint flat_texture_array;
static GLuint* wall_textures;

static camera cam;
static vec2 last_mouse_pos;
static mesh quad_mesh;

static draw_list* d_list;

void engine_init(wad* wad, const char* mapname)
{
	cam = (camera){
		.position = {0.0f, 0.0f, 3.0f},
		.yaw = M_PI_2,
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

	wad_read_playpal(&pal, wad);
	GLuint palette_texture = palette_generate_texture(&pal);
	renderer_set_palette_texture(palette_texture);

	flat_tex* flats = wad_read_flats(&num_flats, wad);
	flat_texture_array = generate_flat_texture_array(flats, num_flats);
	free(flats);

	wall_tex* textures = wad_read_textures(&num_wall_textures, "TEXTURE1", wad);
	wall_textures = malloc(sizeof(GLuint) * num_wall_textures);
	for (int i = 0; i < num_wall_textures; i++)
		wall_textures[i] = generate_texture(textures[i].width, textures[i].height, textures[i].data);

	generate_meshes(&map, &gl_map);
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

	renderer_set_draw_texture(flat_texture_array);
	for (draw_node* node = d_list; node != NULL; node = node->next)
		renderer_draw_mesh(&node->mesh, mat4_identity());
}

static void generate_meshes(const map* map, const gl_map* gl_map)
{
	draw_node** draw_node_ptr = &d_list;
	for (int i = 0; i < gl_map->num_subsectors; i++)
	{
		draw_node* floor_node = malloc(sizeof(draw_node));
		floor_node->next = NULL;
		*draw_node_ptr = floor_node;
		draw_node_ptr = &floor_node->next;

		draw_node* ceil_node = malloc(sizeof(draw_node));
		ceil_node->next = NULL;
		*draw_node_ptr = ceil_node;
		draw_node_ptr = &ceil_node->next;

		sector* sector = NULL;
		gl_subsector* subsector = &gl_map->subsectors[i];
		size_t n_vertices = subsector->num_segs;
		vertex* floor_vertices = malloc(sizeof(vertex) * n_vertices);
		vertex* ceil_vertices = malloc(sizeof(vertex) * n_vertices);

		for (int j = 0; j < subsector->num_segs; j++)
		{
			gl_segment* segment = &gl_map->segments[j + subsector->first_seg];

			if (sector == NULL && segment->linedef != 0xffff)
			{
				linedef* linedef = &map->linedefs[segment->linedef];
				int sector_index = -1;
				if (linedef->flags & LINEDEF_FLAGS_TWO_SIDED && segment->side == 1)
					sector_index = map->sidedefs[linedef->back_sidedef].sector_index;
				else
					sector_index = map->sidedefs[linedef->front_sidedef].sector_index;

				if (sector_index >= 0)
					sector = &map->sectors[sector_index];
			}

			vec2 v;
			if (segment->start_vertex & VERT_IS_GL)
				v = gl_map->vertices[segment->start_vertex & 0x7fff];
			else
				v = map->vertices[segment->start_vertex];

			floor_vertices[j] = ceil_vertices[j] = (vertex){
				.position = {v.x / SCALE, 0.0f, -v.y / SCALE},
				.tex_coords = {v.x / FLAT_TEXTURE_SIZE, -v.y / FLAT_TEXTURE_SIZE},
				.texture_type = 1
			};
		}

		for (int i = 0; i < n_vertices; i++)
		{
			int floor_tex = sector->floor_tex, ceil_tex = sector->ceiling_tex;

			floor_vertices[i].position.y = sector->floor / SCALE;
			floor_vertices[i].texture_index =
				floor_tex >= 0 && floor_tex < num_flats ? floor_tex : -1;

			ceil_vertices[i].position.y = sector->ceiling / SCALE;
			ceil_vertices[i].texture_index =
				ceil_tex >= 0 && ceil_tex < num_flats ? ceil_tex : -1;
		}

		// Triangulate will form (n - 2) triangles so 3 * (n - 2) indices are required
		size_t n_indices = 3 * (n_vertices - 2);
		uint32_t* indices = malloc(sizeof(uint32_t) * n_indices);
		for (int j = 0, k = 1; j < n_indices; j += 3, k++)
		{
			indices[j] = 0;
			indices[j + 1] = k;
			indices[j + 2] = k + 1;
		}

		floor_node->sector = ceil_node->sector = sector;
		mesh_create(&floor_node->mesh, n_vertices, floor_vertices, n_indices, indices);
		mesh_create(&ceil_node->mesh, n_vertices, ceil_vertices, n_indices, indices);

		free(floor_vertices);
		free(ceil_vertices);
		free(indices);
	}

	uint32_t indices[] = {
		0, 1, 3,	// 1st triangle
		1, 2, 3		// 2nd triangle
	};

	for (int i = 0; i < map->num_linedefs; i++)
	{
		linedef* ld = &map->linedefs[i];

		if (ld->flags & LINEDEF_FLAGS_TWO_SIDED)
		{
			// Floor
			draw_node* floor_node = malloc(sizeof(draw_node));
			floor_node->next = NULL;

			vec2 start = map->vertices[ld->start_index];
			vec2 end = map->vertices[ld->end_index];

			sidedef* front_side = &map->sidedefs[ld->front_sidedef];
			sector* front_sect = &map->sectors[front_side->sector_index];
			sidedef* back_side = &map->sidedefs[ld->back_sidedef];
			sector* back_sect = &map->sectors[back_side->sector_index];

			vec3 floor0 = { start.x, front_sect->floor, -start.y };
			vec3 floor1 = { end.x, front_sect->floor, -end.y };
			vec3 floor2 = { end.x, back_sect->floor, -end.y };
			vec3 floor3 = { start.x, back_sect->floor, -start.y };

			floor0 = vec3_scale(floor0, 1.0f / SCALE);
			floor1 = vec3_scale(floor1, 1.0f / SCALE);
			floor2 = vec3_scale(floor2, 1.0f / SCALE);
			floor3 = vec3_scale(floor3, 1.0f / SCALE);

			srand((uintptr_t)front_sect);
			int color = rand() % NUM_COLORS;

			vertex floor_vertices[] = {
				{.position = floor0, .texture_index = color, .texture_type = 0},
				{.position = floor1, .texture_index = color, .texture_type = 0},
				{.position = floor2, .texture_index = color, .texture_type = 0},
				{.position = floor3, .texture_index = color, .texture_type = 0}
			};

			mesh_create(&floor_node->mesh, 4, floor_vertices, 6, indices);
			floor_node->sector = front_sect;

			*draw_node_ptr = floor_node;
			draw_node_ptr = &floor_node->next;

			// Ceiling
			draw_node* ceil_node = malloc(sizeof(draw_node));
			ceil_node->next = NULL;

			vec3 ceil0 = { start.x, front_sect->ceiling, -start.y };
			vec3 ceil1 = { end.x, front_sect->ceiling, -end.y };
			vec3 ceil2 = { end.x, back_sect->ceiling, -end.y };
			vec3 ceil3 = { start.x, back_sect->ceiling, -start.y };

			ceil0 = vec3_scale(ceil0, 1.f / SCALE);
			ceil1 = vec3_scale(ceil1, 1.f / SCALE);
			ceil2 = vec3_scale(ceil2, 1.f / SCALE);
			ceil3 = vec3_scale(ceil3, 1.f / SCALE);

			vertex ceil_vertices[] = {
				{.position = ceil0, .texture_index = color, .texture_type = 0},
				{.position = ceil1, .texture_index = color, .texture_type = 0},
				{.position = ceil2, .texture_index = color, .texture_type = 0},
				{.position = ceil3, .texture_index = color, .texture_type = 0}
			};

			mesh_create(&ceil_node->mesh, 4, ceil_vertices, 6, indices);
			ceil_node->sector = front_sect;

			*draw_node_ptr = ceil_node;
			draw_node_ptr = &ceil_node->next;
		}
		else
		{
			draw_node* node = malloc(sizeof(draw_node));
			node->next = NULL;

			vec2 start = map->vertices[ld->start_index];
			vec2 end = map->vertices[ld->end_index];

			sidedef* side = &map->sidedefs[ld->front_sidedef];
			sector* sect = &map->sectors[side->sector_index];

			vec3 p0 = { start.x, sect->floor, -start.y };
			vec3 p1 = { end.x, sect->floor, -end.y };
			vec3 p2 = { end.x, sect->ceiling, -end.y };
			vec3 p3 = { start.x, sect->ceiling, -start.y };

			p0 = vec3_scale(p0, 1.0f / SCALE);
			p1 = vec3_scale(p1, 1.0f / SCALE);
			p2 = vec3_scale(p2, 1.0f / SCALE);
			p3 = vec3_scale(p3, 1.0f / SCALE);

			srand((uintptr_t)sect);
			int color = rand() % NUM_COLORS;

			vertex vertices[] = {
				{.position = p0, .texture_index = color, .texture_type = 0},
				{.position = p1, .texture_index = color, .texture_type = 0},
				{.position = p2, .texture_index = color, .texture_type = 0},
				{.position = p3, .texture_index = color, .texture_type = 0}
			};

			mesh_create(&node->mesh, 4, vertices, 6, indices);
			node->sector = sect;

			*draw_node_ptr = node;
			draw_node_ptr = &node->next;
		}
	}
}
