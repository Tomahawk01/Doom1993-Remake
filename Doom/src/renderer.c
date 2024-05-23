#include "renderer.h"
#include "gl_utilities.h"
#include "math/matrix.h"

#include "glad/glad.h"

#include <math.h>

static void init_skybox();
static void init_shaders();
static void init_projection();

const char* vert_src =
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

const char* frag_src =
	"#version 330 core\n"
	"in vec2 TexCoords;\n"
	"flat in int TexIndex;\n"
	"flat in int TexType;\n"
	"flat in vec2 MaxTexCoords;"
	"in float Light;\n"
	"out vec4 fragColor;\n"
	"uniform usampler2DArray u_flat_tex;\n"
	"uniform usampler2DArray u_wall_tex;\n"
	"uniform sampler1DArray u_palettes;\n"
	"uniform int u_palette_index;\n"
	"void main() {\n"
	"  vec3 color;\n"
	"  if (TexIndex == -1) { discard; }\n"
	"  else if (TexType == 0) {\n"
	"    color = vec3(texelFetch(u_palettes, ivec2(TexIndex, u_palette_index), 0));\n"
	"  } else if (TexType == 1) {\n"
	"    color = vec3(texelFetch(u_palettes, ivec2(int(texture(u_flat_tex, vec3(TexCoords, TexIndex)).r), u_palette_index), 0));\n"
	"  } else if (TexType == 2) {\n"
	"    color = vec3(texelFetch(u_palettes, ivec2(int(texture(u_wall_tex, vec3(fract(TexCoords / MaxTexCoords) * MaxTexCoords, TexIndex)).r), u_palette_index), 0));\n"
	"  }\n"
	"  fragColor = vec4(color * Light, 1.0);\n"
	"}\n";

const char* sky_vert_src =
	"#version 330 core\n"
	"layout (location = 0) in vec3 pos;\n"
	"out vec3 TexCoords;\n"
	"uniform mat4 u_view;\n"
	"uniform mat4 u_projection;\n"
	"void main() {\n"
	"  vec4 position = u_projection * mat4(mat3(u_view)) * vec4(pos, 1.0);\n"
	"  TexCoords = pos;\n"
	"  gl_Position = position.xyww;\n"
	"}\n";

const char* sky_frag_src =
	"#version 330 core\n"
	"in vec3 TexCoords;\n"
	"out vec4 fragColor;\n"
	"uniform sampler1DArray u_palettes;\n"
	"uniform usamplerCube u_sky;\n"
	"uniform int u_palette_index;\n"
	"void main() {\n"
	"  fragColor = texelFetch(u_palettes, ivec2(int(texture(u_sky, TexCoords).r), u_palette_index), 0);\n"
	"}\n";

unsigned int skybox_vao, skybox_vbo;
static float width;
static float height;
static GLuint program, sky_program;
static GLuint model_location;
static GLuint view_location;
static GLuint projection_location;
static GLuint sky_view_location;
static GLuint sky_projection_location;
static GLuint palette_index_location;
static GLuint sky_palette_index_location;

void renderer_init(int w, int h)
{
	width = w;
	height = h;

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	init_skybox();
	init_shaders();
	init_projection();
}

void renderer_clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_set_palette_texture(GLuint palette_texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D_ARRAY, palette_texture);
}

void renderer_set_palette_index(int index)
{
	glUseProgram(sky_program);
	glUniform1i(sky_palette_index_location, index);

	glUseProgram(program);
	glUniform1i(palette_index_location, index);
}

void renderer_set_wall_texture(GLuint texture)
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void renderer_set_sky_texture(GLuint texture)
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
}

void renderer_set_flat_texture(GLuint texture)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void renderer_set_projection(mat4 projection)
{
	glUseProgram(sky_program);
	glUniformMatrix4fv(sky_projection_location, 1, GL_FALSE, projection.v);

	glUseProgram(program);
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.v);
}

void renderer_set_view(mat4 view)
{
	glUseProgram(sky_program);
	glUniformMatrix4fv(sky_view_location, 1, GL_FALSE, view.v);

	glUseProgram(program);
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

void renderer_draw_sky()
{
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glUseProgram(sky_program);
	glBindVertexArray(skybox_vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	glUseProgram(program);
}

void init_shaders()
{
	GLuint sky_vertex = compile_shader(GL_VERTEX_SHADER, sky_vert_src);
	GLuint sky_fragment = compile_shader(GL_FRAGMENT_SHADER, sky_frag_src);
	sky_program = link_shader(2, sky_vertex, sky_fragment);
	glUseProgram(sky_program);

	sky_projection_location = glGetUniformLocation(sky_program, "u_projection");
	sky_view_location = glGetUniformLocation(sky_program, "u_view");
	sky_palette_index_location = glGetUniformLocation(sky_program, "u_palette_index");

	GLuint sky_palette_location = glGetUniformLocation(sky_program, "u_palettes");
	glUniform1i(sky_palette_location, 0);

	GLuint sky_texture_location = glGetUniformLocation(sky_program, "u_sky");
	glUniform1i(sky_texture_location, 3);

	GLuint vertex = compile_shader(GL_VERTEX_SHADER, vert_src);
	GLuint fragment = compile_shader(GL_FRAGMENT_SHADER, frag_src);
	program = link_shader(2, vertex, fragment);
	glUseProgram(program);

	projection_location = glGetUniformLocation(program, "u_projection");
	model_location = glGetUniformLocation(program, "u_model");
	view_location = glGetUniformLocation(program, "u_view");
	palette_index_location = glGetUniformLocation(program, "u_palette_index");

	GLuint palette_location = glGetUniformLocation(program, "u_palettes");
	glUniform1i(palette_location, 0);

	GLuint flat_texture_location = glGetUniformLocation(program, "u_flat_tex");
	glUniform1i(flat_texture_location, 1);

	GLuint wall_texture_location = glGetUniformLocation(program, "u_wall_tex");
	glUniform1i(wall_texture_location, 2);
}

void init_skybox()
{
	float vertices[] = {
		-1.0f, 1.0f,-1.0f,-1.0f,-1.0f,-1.0f, 1.0f,-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f, 1.0f,-1.0f,-1.0f, 1.0f,-1.0f,-1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,-1.0f,-1.0f, 1.0f,-1.0f,-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,-1.0f,-1.0f, 1.0f, 1.0f,-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,-1.0f,-1.0f,-1.0f, 1.0f,-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, 1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,-1.0f,-1.0f, 1.0f, 1.0f,-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f
	};

	glGenVertexArrays(1, &skybox_vao);
	glGenBuffers(1, &skybox_vbo);
	glBindVertexArray(skybox_vao);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

static void init_projection()
{
	mat4 projection = mat4_ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	GLuint projection_location = glGetUniformLocation(program, "u_projection");
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.v);
	glUniformMatrix4fv(view_location, 1, GL_FALSE, mat4_identity().v);
}
