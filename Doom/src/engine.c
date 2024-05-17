#include "engine.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "map.h"
#include "palette.h"
#include "darray.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FOV (M_PI / 2.0f)	// 90 degrees
#define PLAYER_SPEED (500.0f)
#define MOUSE_SENSITIVITY (0.002f) // in radians

typedef struct draw_node
{
	int texture;
	mesh mesh;
	struct draw_node* next;
} draw_node, draw_list;

typedef struct wall_tex_info
{
	int width;
	int height;
} wall_tex_info;

static void generate_meshes(const map* map, const gl_map* gl_map);
static sector* map_get_sector(map* map, gl_map* gl_map, vec2 position);

static size_t num_flats;
static size_t num_wall_textures;
static size_t num_palettes;
static wall_tex_info* wall_textures_info;
static vec2* wall_max_coords;

static camera cam;
static vec2 last_mouse_pos;

static map m;
static gl_map gl_m;
static float player_height;
static int sky_flat;

static draw_list* d_list;

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

	flat_tex* flats = wad_read_flats(&num_flats, wad);
	GLuint flat_texture_array = generate_flat_texture_array(flats, num_flats);
	free(flats);

	wall_tex* textures = wad_read_textures(&num_wall_textures, "TEXTURE1", wad);
	wall_textures_info = malloc(sizeof(wall_tex_info) * num_wall_textures);
	wall_max_coords = malloc(sizeof(vec2) * num_wall_textures);
	for (int i = 0; i < num_wall_textures; i++)
		wall_textures_info[i] = (wall_tex_info){ textures[i].width, textures[i].height };

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
				.position = {thing->position.x, player_height, -thing->position.y},
				.yaw = thing->angle,
				.pitch = 0.0f
			};
		}
	}

	generate_meshes(&m, &gl_m);

	renderer_set_flat_texture(flat_texture_array);
	renderer_set_wall_texture(wall_texture_array);
	renderer_set_palette_texture(palette_texture);
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

	vec2 position = { cam.position.x, -cam.position.z };
	sector* sector = map_get_sector(&m, &gl_m, position);
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
		cam.yaw += turn_speed;
	if (is_button_pressed(KEY_LEFT))
		cam.yaw -= turn_speed;

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
			last_mouse_pos = get_mouse_position();
			is_first = false;
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
	else
	{
		is_first = true;
	}
}

void engine_render()
{
	mat4 view = mat4_look_at(cam.position, vec3_add(cam.position, cam.forward), cam.up);
	renderer_set_view(view);

	renderer_set_palette_index(palette_index);
	for (draw_node* node = d_list; node != NULL; node = node->next)
		renderer_draw_mesh(&node->mesh, mat4_identity());
}

sector* map_get_sector(map* map, gl_map* gl_map, vec2 position)
{
	uint16_t id = gl_map->num_nodes - 1;
	while ((id & 0x8000) == 0)
	{
		if (id > gl_map->num_nodes)
			return NULL;

		gl_node* node = &gl_map->nodes[id];

		vec2 delta = vec2_sub(position, node->partition);
		bool is_on_back = (delta.x * node->delta_partition.y - delta.y * node->delta_partition.x) <= 0.0f;

		if (is_on_back)
			id = gl_map->nodes[id].back_child_id;
		else
			id = gl_map->nodes[id].front_child_id;
	}

	if ((id & 0x7fff) >= gl_map->num_subsectors)
		return NULL;

	gl_subsector* subsector = &gl_map->subsectors[id & 0x7fff];
	gl_segment* segment = &gl_map->segments[subsector->first_seg];
	linedef* linedef = &map->linedefs[segment->linedef];

	sidedef* sidedef;
	if (segment->side == 0)
		sidedef = &map->sidedefs[linedef->front_sidedef];
	else
		sidedef = &map->sidedefs[linedef->back_sidedef];

	return &map->sectors[sidedef->sector_index];
}

void generate_meshes(const map* map, const gl_map* gl_map)
{
	draw_node** draw_node_ptr = &d_list;

	draw_node* flats_node = malloc(sizeof(draw_node));
	flats_node->next = NULL;
	flats_node->texture = -1;

	vertexarray vertices;
	indexarray indices;
	darray_init(vertices, 0);
	darray_init(indices, 0);

	int start_idx = 0;
	for (int i = 0; i < gl_map->num_subsectors; i++)
	{
		sector* sector = NULL;
		gl_subsector* subsector = &gl_map->subsectors[i];
		size_t n_vertices = subsector->num_segs;
		if (n_vertices < 3)
			continue;

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
				.position = {v.x, 0.0f, -v.y},
				.tex_coords = {v.x / FLAT_TEXTURE_SIZE, -v.y / FLAT_TEXTURE_SIZE},
				.texture_type = 1
			};
		}

		for (int i = 0; i < n_vertices; i++)
		{
			int floor_tex = sector->floor_tex;
			int ceil_tex = sector->ceiling_tex;

			floor_vertices[i].position.y = sector->floor;
			floor_vertices[i].texture_index = floor_tex >= 0 && floor_tex < num_flats ? floor_tex : -1;

			ceil_vertices[i].position.y = sector->ceiling;
			ceil_vertices[i].texture_index = ceil_tex >= 0 && ceil_tex < num_flats ? ceil_tex : -1;

			floor_vertices[i].light = ceil_vertices[i].light = sector->light_level / 256.0f;
		}

		for (int i = 0; i < n_vertices; i++)
			darray_push(vertices, floor_vertices[i]);

		int ceil_start_idx = vertices.count;
		for (int i = 0; i < n_vertices; i++)
			darray_push(vertices, ceil_vertices[i]);

		// Triangulation will form (n - 2) triangles so 2 * 3 * (n - 3) indices are required
		for (int j = 0, k = 1; j < n_vertices - 2; j++, k++)
		{
			darray_push(indices, start_idx);
			darray_push(indices, start_idx + k);
			darray_push(indices, start_idx + k + 1);

			darray_push(indices, ceil_start_idx);
			darray_push(indices, ceil_start_idx + k);
			darray_push(indices, ceil_start_idx + k + 1);
		}

		start_idx = vertices.count;
		free(floor_vertices);
		free(ceil_vertices);
	}

	mesh_create(&flats_node->mesh, vertices.count, vertices.data, indices.count, indices.data);
	*draw_node_ptr = flats_node;
	draw_node_ptr = &flats_node->next;

	darray_free(vertices);
	darray_free(indices);

	uint32_t quad_indices[] = {
		0, 1, 3,	// 1st triangle
		1, 2, 3		// 2nd triangle
	};

	for (int i = 0; i < gl_map->num_segments; i++)
	{
		gl_segment* segment = &gl_map->segments[i];
		if (segment->linedef == 0xffff) 
			continue;

		linedef* ld = &map->linedefs[segment->linedef];

		vec2 start;
		vec2 end;
		if (segment->start_vertex & VERT_IS_GL)
			start = gl_map->vertices[segment->start_vertex & 0x7fff];
		else
			start = map->vertices[segment->start_vertex];

		if (segment->end_vertex & VERT_IS_GL)
			end = gl_map->vertices[segment->end_vertex & 0x7fff];
		else
			end = map->vertices[segment->end_vertex];

		sidedef* front_sidedef = &map->sidedefs[ld->front_sidedef];
		sidedef* back_sidedef = &map->sidedefs[ld->back_sidedef];

		if (segment->side)
		{
			sidedef* tmp = front_sidedef;
			front_sidedef = back_sidedef;
			back_sidedef = tmp;
		}

		sector* front_sector = &map->sectors[front_sidedef->sector_index];
		sector* back_sector = &map->sectors[back_sidedef->sector_index];

		sidedef* sidedef = front_sidedef;
		sector* sector = front_sector;

		if (ld->flags & LINEDEF_FLAGS_TWO_SIDED)
		{
			if (sidedef->lower >= 0)
			{
				draw_node* floor_node = malloc(sizeof(draw_node));
				floor_node->next = NULL;

				vec3 p0 = { start.x, front_sector->floor, -start.y };
				vec3 p1 = { end.x, front_sector->floor, -end.y };
				vec3 p2 = { end.x, back_sector->floor, -end.y };
				vec3 p3 = { start.x, back_sector->floor, -start.y };

				const float x = p1.x - p0.x;
				const float y = p1.z - p0.z;
				const float width = sqrtf(x * x + y * y);
				const float height = fabsf(p3.y - p0.y);

				float tw = wall_textures_info[sidedef->lower].width;
				float th = wall_textures_info[sidedef->lower].height;

				float w = width / tw;
				float h = height / th;
				float x_off = sidedef->x_off / tw;
				float y_off = sidedef->y_off / th;
				if (ld->flags & LINEDEF_FLAGS_LOWER_UNPEGGED)
					y_off += (front_sector->ceiling - back_sector->floor) / th;

				float tx0 = x_off;
				float ty0 = y_off + h;
				float tx1 = x_off + w;
				float ty1 = y_off;

				vec2 max_coords = wall_max_coords[sidedef->lower];
				tx0 *= max_coords.x, tx1 *= max_coords.x;
				ty0 *= max_coords.y, ty1 *= max_coords.y;

				float light = front_sector->light_level / 256.0f;
				vertex vertices[] = {
					{p0, {tx0, ty0}, sidedef->lower, 2, light, max_coords},
					{p1, {tx1, ty0}, sidedef->lower, 2, light, max_coords},
					{p2, {tx1, ty1}, sidedef->lower, 2, light, max_coords},
					{p3, {tx0, ty1}, sidedef->lower, 2, light, max_coords}
				};

				mesh_create(&floor_node->mesh, 4, vertices, 6, quad_indices);
				*draw_node_ptr = floor_node;
				draw_node_ptr = &floor_node->next;
			}

			if (sidedef->upper >= 0 && !(front_sector->ceiling_tex == sky_flat && back_sector->ceiling_tex == sky_flat))
			{
				draw_node* ceil_node = malloc(sizeof(draw_node));
				ceil_node->next = NULL;

				vec3 p0 = { start.x, front_sector->ceiling, -start.y };
				vec3 p1 = { end.x, front_sector->ceiling, -end.y };
				vec3 p2 = { end.x, back_sector->ceiling, -end.y };
				vec3 p3 = { start.x, back_sector->ceiling, -start.y };

				const float x = p1.x - p0.x;
				const float y = p1.z - p0.z;
				const float width = sqrtf(x * x + y * y);
				const float height = -fabsf(p3.y - p0.y);

				float tw = wall_textures_info[sidedef->upper].width;
				float th = wall_textures_info[sidedef->upper].height;

				float w = width / tw;
				float h = height / th;
				float x_off = sidedef->x_off / tw;
				float y_off = sidedef->y_off / th;
				if (ld->flags & LINEDEF_FLAGS_UPPER_UNPEGGED)
					y_off -= h;

				float tx0 = x_off;
				float ty0 = y_off + h;
				float tx1 = x_off + w;
				float ty1 = y_off;

				vec2 max_coords = wall_max_coords[sidedef->upper];
				tx0 *= max_coords.x, tx1 *= max_coords.x;
				ty0 *= max_coords.y, ty1 *= max_coords.y;

				float light = front_sector->light_level / 256.0f;
				vertex vertices[] = {
					{p0, {tx0, ty0}, sidedef->upper, 2, light, max_coords},
					{p1, {tx1, ty0}, sidedef->upper, 2, light, max_coords},
					{p2, {tx1, ty1}, sidedef->upper, 2, light, max_coords},
					{p3, {tx0, ty1}, sidedef->upper, 2, light, max_coords}
				};

				mesh_create(&ceil_node->mesh, 4, vertices, 6, quad_indices);
				*draw_node_ptr = ceil_node;
				draw_node_ptr = &ceil_node->next;
			}
		}
		else if (sidedef->middle >= 0)
		{
			draw_node* node = malloc(sizeof(draw_node));
			node->next = NULL;

			vec3 p0 = { start.x, sector->floor, -start.y };
			vec3 p1 = { end.x, sector->floor, -end.y };
			vec3 p2 = { end.x, sector->ceiling, -end.y };
			vec3 p3 = { start.x, sector->ceiling, -start.y };

			const float x = p1.x - p0.x;
			const float y = p1.z - p0.z;
			const float width = sqrtf(x * x + y * y);
			const float height = p3.y - p0.y;

			float tw = wall_textures_info[sidedef->middle].width;
			float th = wall_textures_info[sidedef->middle].height;

			float w = width / tw;
			float h = height / th;

			float x_off = sidedef->x_off / tw;
			float y_off = sidedef->y_off / th;
			if (ld->flags & LINEDEF_FLAGS_LOWER_UNPEGGED)
				y_off -= h;

			float tx0 = x_off;
			float ty0 = y_off + h;
			float tx1 = x_off + w;
			float ty1 = y_off;

			vec2 max_coords = wall_max_coords[sidedef->middle];
			tx0 *= max_coords.x, tx1 *= max_coords.x;
			ty0 *= max_coords.y, ty1 *= max_coords.y;

			float light = sector->light_level / 256.0f;
			vertex vertices[] = {
				{p0, {tx0, ty0}, sidedef->middle, 2, light, max_coords},
				{p1, {tx1, ty0}, sidedef->middle, 2, light, max_coords},
				{p2, {tx1, ty1}, sidedef->middle, 2, light, max_coords},
				{p3, {tx0, ty1}, sidedef->middle, 2, light, max_coords}
			};

			mesh_create(&node->mesh, 4, vertices, 6, quad_indices);

			*draw_node_ptr = node;
			draw_node_ptr = &node->next;
		}
	}
}
