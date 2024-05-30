#include "engine/meshgen.h"
#include "engine/state.h"
#include "engine/utilities.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "darray.h"
#include "texture/flat_texture.h"
#include "gl_map.h"
#include "map.h"

#define _USE_MATH_DEFINES
#include <math.h>

static void generate_node(draw_node** draw_node_ptr, size_t id);

void generate_meshes()
{
	max_sector_height = 0.0f;
	for (int i = 0; i < m.num_sectors; i++)
	{
		if (m.sectors[i].ceiling > max_sector_height)
			max_sector_height = m.sectors[i].ceiling;
	}
	max_sector_height += 1.0f;

	float width = m.max.x - m.min.x;
	float height = m.max.y - m.min.y;
	vec3 translate = { m.min.x, max_sector_height, m.max.y };

	mat4 scale = mat4_scale((vec3) { width, height, 1.0f });
	mat4 translation = mat4_translate(translate);
	mat4 rotation = mat4_rotate((vec3) { 1.0f, 0.0f, 0.0f }, M_PI / 2.0f);
	mat4 model = mat4_mult(scale, mat4_mult(rotation, translation));
	insert_stencil_quad(model);

	generate_node(&root_draw_node, gl_m.num_nodes - 1);
}

void generate_node(draw_node** draw_node_ptr, size_t id)
{
	draw_node* d_node = malloc(sizeof(draw_node));
	*d_node = (draw_node){ NULL, NULL, NULL };
	*draw_node_ptr = d_node;

	if (id & 0x8000)
	{
		gl_subsector* subsector = &gl_m.subsectors[id & 0x7fff];

		vertexarray vertices;
		indexarray indices;
		darray_init(vertices, 0);
		darray_init(indices, 0);

		sector* the_sector = NULL;
		size_t n_vertices = subsector->num_segs;
		if (n_vertices < 3)
			return;

		vertex* floor_vertices = malloc(sizeof(vertex) * n_vertices);
		vertex* ceil_vertices = malloc(sizeof(vertex) * n_vertices);

		size_t start_index = 0;

		for (int j = 0; j < subsector->num_segs; j++)
		{
			gl_segment* segment = &gl_m.segments[j + subsector->first_seg];

			vec2 start, end;
			if (segment->start_vertex & VERT_IS_GL)
				start = gl_m.vertices[segment->start_vertex & 0x7fff];
			else
				start = m.vertices[segment->start_vertex];

			if (segment->end_vertex & VERT_IS_GL)
				end = gl_m.vertices[segment->end_vertex & 0x7fff];
			else
				end = m.vertices[segment->end_vertex];

			if (the_sector == NULL && segment->linedef != 0xffff)
			{
				linedef* linedef = &m.linedefs[segment->linedef];
				int sector_index = -1;
				if (linedef->flags & LINEDEF_FLAGS_TWO_SIDED && segment->side == 1)
					sector_index = m.sidedefs[linedef->back_sidedef].sector_index;
				else
					sector_index = m.sidedefs[linedef->front_sidedef].sector_index;

				if (sector_index >= 0)
					the_sector = &m.sectors[sector_index];
			}

			floor_vertices[j] = ceil_vertices[j] = (vertex){
				.position = {start.x, 0.0f, start.y},
				.tex_coords = {start.x / FLAT_TEXTURE_SIZE, start.y / FLAT_TEXTURE_SIZE},
				.texture_type = 1
			};

			if (segment->linedef == 0xffff)
				continue;

			linedef* linedef = &m.linedefs[segment->linedef];

			sidedef* front_sidedef = &m.sidedefs[linedef->front_sidedef];
			sidedef* back_sidedef = &m.sidedefs[linedef->back_sidedef];

			if (segment->side)
			{
				sidedef* tmp = front_sidedef;
				front_sidedef = back_sidedef;
				back_sidedef = tmp;
			}

			sector* front_sector = &m.sectors[front_sidedef->sector_index];
			sector* back_sector = &m.sectors[back_sidedef->sector_index];

			sidedef* sidedef = front_sidedef;
			sector* sector = front_sector;

			if (linedef->flags & LINEDEF_FLAGS_TWO_SIDED)
			{
				if (sidedef->lower >= 0 && front_sector->floor < back_sector->floor)
				{
					vec3 p0 = { start.x, front_sector->floor, start.y };
					vec3 p1 = { end.x, front_sector->floor, end.y };
					vec3 p2 = { end.x, back_sector->floor, end.y };
					vec3 p3 = { start.x, back_sector->floor, start.y };

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

					if (linedef->flags & LINEDEF_FLAGS_LOWER_UNPEGGED)
						y_off += (front_sector->ceiling - back_sector->floor) / th;

					float tx0 = x_off;
					float ty0 = y_off + h;
					float tx1 = x_off + w;
					float ty1 = y_off;

					vec2 max_coords = wall_max_coords[sidedef->lower];
					tx0 *= max_coords.x, tx1 *= max_coords.x;
					ty0 *= max_coords.y, ty1 *= max_coords.y;

					float light = front_sector->light_level / 256.0f;
					vertex v[] = {
						{p0, {tx0, ty0}, sidedef->lower, 2, light, max_coords},
						{p1, {tx1, ty0}, sidedef->lower, 2, light, max_coords},
						{p2, {tx1, ty1}, sidedef->lower, 2, light, max_coords},
						{p3, {tx0, ty1}, sidedef->lower, 2, light, max_coords}
					};

					start_index = vertices.count;
					for (int i = 0; i < 4; i++)
						darray_push(vertices, v[i]);

					darray_push(indices, start_index + 0);
					darray_push(indices, start_index + 1);
					darray_push(indices, start_index + 3);
					darray_push(indices, start_index + 1);
					darray_push(indices, start_index + 2);
					darray_push(indices, start_index + 3);
				}

				if (sidedef->upper >= 0 && front_sector->ceiling > back_sector->ceiling && !(front_sector->ceiling_tex == sky_flat && back_sector->ceiling_tex == sky_flat))
				{
					vec3 p0 = { start.x, back_sector->ceiling, start.y };
					vec3 p1 = { end.x, back_sector->ceiling, end.y };
					vec3 p2 = { end.x, front_sector->ceiling, end.y };
					vec3 p3 = { start.x, front_sector->ceiling, start.y };

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

					if (linedef->flags & LINEDEF_FLAGS_UPPER_UNPEGGED)
						y_off -= h;

					float tx0 = x_off;
					float ty0 = y_off;
					float tx1 = x_off + w;
					float ty1 = y_off + h;

					vec2 max_coords = wall_max_coords[sidedef->upper];
					tx0 *= max_coords.x, tx1 *= max_coords.x;
					ty0 *= max_coords.y, ty1 *= max_coords.y;

					float light = front_sector->light_level / 256.0f;
					vertex v[] = {
						{p0, {tx0, ty0}, sidedef->upper, 2, light, max_coords},
						{p1, {tx1, ty0}, sidedef->upper, 2, light, max_coords},
						{p2, {tx1, ty1}, sidedef->upper, 2, light, max_coords},
						{p3, {tx0, ty1}, sidedef->upper, 2, light, max_coords},
					};

					start_index = vertices.count;
					for (int i = 0; i < 4; i++)
						darray_push(vertices, v[i]);

					darray_push(indices, start_index + 0);
					darray_push(indices, start_index + 1);
					darray_push(indices, start_index + 3);
					darray_push(indices, start_index + 1);
					darray_push(indices, start_index + 2);
					darray_push(indices, start_index + 3);

					if (sector->ceiling_tex == sky_flat)
					{
						float quad_height = max_sector_height - p3.y;
						mat4 scale = mat4_scale((vec3) { width, quad_height, 1.0f });
						mat4 translation = mat4_translate(p3);
						mat4 rotation = mat4_rotate((vec3) { 0.0f, 1.0f, 0.0f }, atan2f(y, x));
						mat4 model = mat4_mult(scale, mat4_mult(rotation, translation));

						insert_stencil_quad(model);
					}
				}
			}
			else
			{
				vec3 p0 = { start.x, sector->floor, start.y };
				vec3 p1 = { end.x, sector->floor, end.y };
				vec3 p2 = { end.x, sector->ceiling, end.y };
				vec3 p3 = { start.x, sector->ceiling, start.y };

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

				if (linedef->flags & LINEDEF_FLAGS_LOWER_UNPEGGED)
					y_off -= h;

				float tx0 = x_off, ty0 = y_off + h;
				float tx1 = x_off + w, ty1 = y_off;

				vec2 max_coords = wall_max_coords[sidedef->middle];
				tx0 *= max_coords.x, tx1 *= max_coords.x;
				ty0 *= max_coords.y, ty1 *= max_coords.y;

				float light = sector->light_level / 256.0f;
				vertex v[] = {
					{p0, {tx0, ty0}, sidedef->middle, 2, light, max_coords},
					{p1, {tx1, ty0}, sidedef->middle, 2, light, max_coords},
					{p2, {tx1, ty1}, sidedef->middle, 2, light, max_coords},
					{p3, {tx0, ty1}, sidedef->middle, 2, light, max_coords},
				};

				start_index = vertices.count;
				for (int i = 0; i < 4; i++)
					darray_push(vertices, v[i]);

				darray_push(indices, start_index + 0);
				darray_push(indices, start_index + 1);
				darray_push(indices, start_index + 3);
				darray_push(indices, start_index + 1);
				darray_push(indices, start_index + 2);
				darray_push(indices, start_index + 3);

				if (sector->ceiling_tex == sky_flat)
				{
					float quad_height = max_sector_height - p3.y;
					mat4 scale = mat4_scale((vec3) { width, quad_height, 1.0f });
					mat4 translation = mat4_translate(p3);
					mat4 rotation = mat4_rotate((vec3) { 0.0f, 1.0f, 0.0f }, atan2f(y, x));
					mat4 model = mat4_mult(scale, mat4_mult(rotation, translation));

					insert_stencil_quad(model);
				}
			}
		}

		start_index = vertices.count;
		for (int i = 0; i < n_vertices; i++)
		{
			int floor_tex = the_sector->floor_tex;
			int ceil_tex = the_sector->ceiling_tex;

			floor_vertices[i].position.y = the_sector->floor;
			floor_vertices[i].texture_index = floor_tex >= 0 && floor_tex < num_flats ? floor_tex : -1;

			ceil_vertices[i].position.y = the_sector->ceiling;
			ceil_vertices[i].texture_index = ceil_tex >= 0 && ceil_tex < num_flats ? ceil_tex : -1;

			floor_vertices[i].light = ceil_vertices[i].light = the_sector->light_level / 256.0f;
		}

		for (int i = 0; i < n_vertices; i++)
			darray_push(vertices, floor_vertices[i]);

		for (int i = 0; i < n_vertices; i++)
			darray_push(vertices, ceil_vertices[i]);

		// Triangulation will form (n - 2) triangles so 2 * 3 * (n - 2) indices are required
		uint32_t* stencil_indices = malloc(sizeof(uint32_t) * 3 * (n_vertices - 2));
		for (int j = 0, k = 1; j < n_vertices - 2; j++, k++)
		{
			darray_push(indices, start_index + 0);
			darray_push(indices, start_index + k + 1);
			darray_push(indices, start_index + k);

			darray_push(indices, start_index + n_vertices);
			darray_push(indices, start_index + n_vertices + k);
			darray_push(indices, start_index + n_vertices + k + 1);

			stencil_indices[3 * j + 0] = 0;
			stencil_indices[3 * j + 1] = k;
			stencil_indices[3 * j + 2] = k + 1;
		}

		if (the_sector->ceiling_tex == sky_flat)
		{
			vec3* stencil_vertices = malloc(sizeof(vec3) * n_vertices);
			for (int i = 0; i < n_vertices; i++)
				stencil_vertices[i] = ceil_vertices[i].position;

			mesh* stencil_mesh = malloc(sizeof(mesh));
			mesh_create(stencil_mesh, VERTEX_LAYOUT_PLAIN, n_vertices, stencil_vertices, 3 * (n_vertices - 2), stencil_indices);

			free(stencil_vertices);
		}

		free(stencil_indices);
		free(floor_vertices);
		free(ceil_vertices);

		d_node->mesh = malloc(sizeof(mesh));
		mesh_create(d_node->mesh, VERTEX_LAYOUT_FULL, vertices.count, vertices.data, indices.count, indices.data);
		darray_free(vertices);
		darray_free(indices);
	}
	else
	{
		gl_node* node = &gl_m.nodes[id];
		generate_node(&d_node->front, node->front_child_id);
		generate_node(&d_node->back, node->back_child_id);
	}
}
