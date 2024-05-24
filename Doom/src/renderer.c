#include "renderer.h"
#include "gl_utilities.h"
#include "math/matrix.h"

#include "glad/glad.h"

#include <math.h>

static void init_skybox();
static void init_shaders();

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

const char* plain_vert_src =
	"#version 330 core\n"
	"layout (location = 0) in vec3 pos;\n"
	"uniform mat4 u_model;\n"
	"uniform mat4 u_view;\n"
	"uniform mat4 u_projection;\n"
	"void main() {\n"
	"  gl_Position = u_projection * u_view * u_model * vec4(pos, 1.0);\n"
	"}\n";

const char* plain_frag_src =
	"#version 330 core\n"
	"out vec4 fragColor;\n"
	"void main() {\n"
	"  fragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
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

static struct
{
	GLuint id;
	GLint model_location, view_location, projection_location;
	GLint palette_index_location;
} shaders[NUM_SHADERS];

static GLuint skybox_vao, skybox_vbo;
static float width;
static float height;

void renderer_init(int w, int h)
{
	width = w;
	height = h;

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	init_skybox();
	init_shaders();
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
	for (int i = 0; i < NUM_SHADERS; i++)
	{
		glUseProgram(shaders[i].id);
		if (shaders[i].palette_index_location != -1)
			glUniform1i(shaders[i].palette_index_location, index);
	}
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
	for (int i = 0; i < NUM_SHADERS; i++)
	{
		glUseProgram(shaders[i].id);
		if (shaders[i].projection_location != -1)
			glUniformMatrix4fv(shaders[i].projection_location, 1, GL_FALSE, projection.v);
	}
}

void renderer_set_view(mat4 view)
{
	for (int i = 0; i < NUM_SHADERS; i++)
	{
		glUseProgram(shaders[i].id);
		if (shaders[i].view_location != -1)
			glUniformMatrix4fv(shaders[i].view_location, 1, GL_FALSE, view.v);
	}
}

vec2 renderer_get_size()
{
	return (vec2) { width, height };
}

void renderer_draw_mesh(const mesh* mesh, int shader, mat4 transformation)
{
	glUseProgram(shaders[shader].id);
	glUniformMatrix4fv(shaders[shader].model_location, 1, GL_FALSE, transformation.v);

	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);
}

void renderer_draw_sky()
{
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glUseProgram(shaders[SHADER_SKY].id);
	glBindVertexArray(skybox_vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

void init_shaders()
{
	struct
	{
		const char* vert;
		const char* frag;
	} shader_units[NUM_SHADERS] = {
		[SHADER_DEFAULT] = {vert_src, frag_src},
		[SHADER_SKY] = {sky_vert_src, sky_frag_src}
	};

	for (int i = 0; i < NUM_SHADERS; i++)
	{
		GLuint vertex = compile_shader(GL_VERTEX_SHADER, shader_units[i].vert);
		GLuint fragment = compile_shader(GL_FRAGMENT_SHADER, shader_units[i].frag);
		shaders[i].id = link_shader(2, vertex, fragment);
		glUseProgram(shaders[i].id);

		shaders[i].projection_location = glGetUniformLocation(shaders[i].id, "u_projection");
		shaders[i].model_location = glGetUniformLocation(shaders[i].id, "u_model");
		shaders[i].view_location = glGetUniformLocation(shaders[i].id, "u_view");
		shaders[i].palette_index_location = glGetUniformLocation(shaders[i].id, "u_palette_index");

		GLint palette_location = glGetUniformLocation(shaders[i].id, "u_palettes");
		if (palette_location != -1)
			glUniform1i(palette_location, 0);

		GLint flat_texture_location = glGetUniformLocation(shaders[i].id, "u_flat_tex");
		if (flat_texture_location != -1)
			glUniform1i(flat_texture_location, 1);

		GLint wall_texture_location = glGetUniformLocation(shaders[i].id, "u_wall_tex");
		if (wall_texture_location != -1)
			glUniform1i(wall_texture_location, 2);

		GLint sky_texture_location = glGetUniformLocation(shaders[i].id, "u_sky");
		if (sky_texture_location != -1)
			glUniform1i(sky_texture_location, 3);
	}
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
