#include "renderer.h"
#include "gl_utilities.h"
#include "math/matrix.h"

#include "glad/glad.h"

#include <math.h>
#include <stdint.h>

static void init_shader();
static void init_quad();
static void init_projection();

const char* vertSrc =
	"#version 330 core\n"
	"layout (location = 0) in vec3 pos;\n"
	"layout (location = 1) in vec2 texCoords;\n"
	"layout (location = 2) in int texIndex;\n"
	"layout (location = 3) in int texType;\n"
	"layout (location = 4) in float light;\n"
	"layout (location = 5) in vec2 maxTexCoords;\n"
	"out vec2 TexCoords;\n"
	"flat out int TexIndex;\n"
	"flat out int TexType;\n"
	"flat out vec2 MaxTexCoords;"
	"out float Light;\n"
	"uniform mat4 u_model;\n"
	"uniform mat4 u_view;\n"
	"uniform mat4 u_projection;\n"
	"void main() {\n"
	"  gl_Position = u_projection * u_view * u_model * vec4(pos, 1.0);\n"
	"  TexIndex = texIndex;\n"
	"  TexType = texType;\n"
	"  TexCoords = texCoords;\n"
	"  Light = light;\n"
	"  MaxTexCoords = maxTexCoords;\n"
	"}\n";

const char* fragSrc =
	"#version 330 core\n"
	"in vec2 TexCoords;\n"
	"flat in int TexIndex;\n"
	"flat in int TexType;\n"
	"flat in vec2 MaxTexCoords;"
	"in float Light;\n"
	"out vec4 fragColor;\n"
	"uniform usampler2DArray u_flat_tex;\n"
	"uniform usampler2DArray u_wall_tex;\n"
	"uniform sampler1D u_palette;\n"
	"void main() {\n"
	"  vec3 color;\n"
	"  if (TexIndex == -1) { discard; }\n"
	"  else if (TexType == 0) {\n"
	"    color = vec3(texelFetch(u_palette, TexIndex, 0));\n"
	"  } else if (TexType == 1) {\n"
	"    color = vec3(texelFetch(u_palette, int(texture(u_flat_tex, vec3(TexCoords, TexIndex)).r), 0));\n"
	"  } else if (TexType == 2) {\n"
	"    color = vec3(texelFetch(u_palette, int(texture(u_wall_tex, vec3(fract(TexCoords / MaxTexCoords) * MaxTexCoords, TexIndex)).r), 0));\n"
	"  }\n"
	"  fragColor = vec4(color * Light, 1.0);\n"
	"}\n";

static mesh quad_mesh;
static float width;
static float height;
static GLuint program;
static GLuint model_location;
static GLuint view_location;
static GLuint projection_location;
static GLuint color_location;

void renderer_init(int w, int h)
{
	width = w;
	height = h;

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	init_quad();
	init_shader();
	init_projection();
}

void renderer_clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_set_palette_texture(GLuint palette_texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, palette_texture);
}

void renderer_set_wall_texture(GLuint texture)
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void renderer_set_flat_texture(GLuint texture)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void renderer_set_projection(mat4 projection)
{
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.v);
}

void renderer_set_view(mat4 view)
{
	glUniformMatrix4fv(view_location, 1, GL_FALSE, view.v);
}

vec2 renderer_get_size()
{
	return (vec2) { width, height };
}

void renderer_draw_mesh(const mesh* mesh, mat4 transformation)
{
	glUniformMatrix4fv(model_location, 1, GL_FALSE, transformation.v);

	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);
}

static void init_shader()
{
	GLuint vertex = compile_shader(GL_VERTEX_SHADER, vertSrc);
	GLuint fragment = compile_shader(GL_FRAGMENT_SHADER, fragSrc);

	program = link_shader(2, vertex, fragment);
	glUseProgram(program);

	projection_location = glGetUniformLocation(program, "u_projection");
	model_location = glGetUniformLocation(program, "u_model");
	view_location = glGetUniformLocation(program, "u_view");
	color_location = glGetUniformLocation(program, "u_color");

	GLuint palette_location = glGetUniformLocation(program, "u_palette");
	glUniform1i(palette_location, 0);

	GLuint texture_array_location = glGetUniformLocation(program, "u_flat_tex");
	glUniform1i(texture_array_location, 1);

	GLuint texture_location = glGetUniformLocation(program, "u_wall_tex");
	glUniform1i(texture_location, 2);
}

static void init_quad()
{
	vertex vertices[] = {
		{.position = { 0.5f, 0.5f, 0.0f}, .tex_coords = {1.0f, 1.0f}},	// top-right
		{.position = { 0.5f,-0.5f, 0.0f}, .tex_coords = {1.0f, 0.0f}},	// bottom-right
		{.position = {-0.5f,-0.5f, 0.0f}, .tex_coords = {0.0f, 0.0f}},  // bottom-left
		{.position = {-0.5f, 0.5f, 0.0f}, .tex_coords = {0.0f, 1.0f}}	// top-left
	};

	uint32_t indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	mesh_create(&quad_mesh, 4, vertices, 6, indices);
}

static void init_projection()
{
	mat4 projection = mat4_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	GLuint projection_location = glGetUniformLocation(program, "u_projection");
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.v);
	glUniformMatrix4fv(view_location, 1, GL_FALSE, mat4_identity().v);
}
