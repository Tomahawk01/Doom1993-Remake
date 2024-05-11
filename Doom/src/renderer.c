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
	"layout(location = 0) in vec2 pos;\n"
	"uniform mat4 u_model;\n"
	"uniform mat4 u_view;\n"
	"uniform mat4 u_projection;\n"
	"void main() {\n"
	"  gl_Position = u_projection * u_view * u_model * vec4(pos, 0.0, 1.0);\n"
	"}\n";

const char* fragSrc =
	"#version 330 core\n"
	"out vec4 fragColor;\n"
	"uniform vec4 u_color;\n"
	"void main() {\n"
	"  fragColor = u_color;\n"
	"}\n";

static mesh quad_mesh;
static float width, height;
static GLuint program;
static GLuint model_location, view_location, projection_location;
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

void renderer_draw_mesh(const mesh* mesh, mat4 transformation, vec4 color)
{
	glUniform4fv(color_location, 1, color.v);
	glUniformMatrix4fv(model_location, 1, GL_FALSE, transformation.v);

	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);
}

void renderer_draw_point(vec2 point, float size, vec4 color)
{
	mat4 translation = mat4_translate((vec3){ point.x, point.y, 0.0f });
	mat4 scale = mat4_scale((vec3) { size, size, 1.0f });
	mat4 model = mat4_mult(scale, translation);

	renderer_draw_mesh(&quad_mesh, model, color);
}

void renderer_draw_line(vec2 p0, vec2 p1, float width, vec4 color)
{
	float x = p1.x - p0.x;
	float y = p0.y - p1.y;
	float r = sqrtf(x * x + y * y);
	float angle = atan2f(y, x);

	mat4 translation = mat4_translate((vec3) { (p0.x + p1.x) / 2.0f, (p0.y + p1.y) / 2.0f, 0.0f });
	mat4 scale = mat4_scale((vec3) { r, width, 1.0f });
	mat4 rotation = mat4_rotate((vec3) { 0.0f, 0.0f, 1.0f }, angle);
	mat4 model = mat4_mult(mat4_mult(scale, rotation), translation);

	renderer_draw_mesh(&quad_mesh, model, color);
}

void renderer_draw_quad(vec2 center, vec2 size, float angle, vec4 color)
{
	mat4 translation = mat4_translate((vec3) { center.x, center.y, 0.0f });
	mat4 scale = mat4_scale((vec3) { size.x, size.y, 1.0f });
	mat4 rotation = mat4_rotate((vec3) { 0.0f, 0.0f, 1.0f }, angle);
	mat4 model = mat4_mult(mat4_mult(scale, rotation), translation);

	renderer_draw_mesh(&quad_mesh, model, color);
}

static void init_shader()
{
	GLuint vertex = compile_shader(GL_VERTEX_SHADER, vertSrc);
	GLuint fragment = compile_shader(GL_FRAGMENT_SHADER, fragSrc);

	program = link_shader(2, vertex, fragment);
	glUseProgram(program);

	model_location = glGetUniformLocation(program, "u_model");
	view_location = glGetUniformLocation(program, "u_view");
	projection_location = glGetUniformLocation(program, "u_projection");
	color_location = glGetUniformLocation(program, "u_color");
}

static void init_quad()
{
	vertex vertices[] = {
		{.position = { 0.5f, 0.5f, 0.0f}},	// top-right
		{.position = { 0.5f,-0.5f, 0.0f}},	// bottom-right
		{.position = {-0.5f,-0.5f, 0.0f}},  // bottom-left
		{.position = {-0.5f, 0.5f, 0.0f}}	// top-left
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
