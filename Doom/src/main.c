#include "renderer.h"
#include "wad_loader.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 1280
#define HEIGHT 720

int main(int argc, char** argv)
{
	if (glfwInit() != GLFW_TRUE)
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Doom1993-Remake", NULL, NULL);
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader(glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to initialize Glad\n");
		return -1;
	}

	wad wad;
	if (wad_load_from_file("res/doom.wad", &wad) != 0)
	{
		printf("Failed to load WAD file\n");
		return -1;
	}

	map map;
	if (wad_read_map("E1M1", &map, &wad) != 0)
	{
		printf("Failed to read map 'E1M1' from WAD file\n");
		return -1;
	}

	vec2 out_min = { 20.0f, 20.0f };
	vec2 out_max = { WIDTH - 20.0f, HEIGHT - 20.0f };
	vec2* remapped_vertices = malloc(sizeof(vec2) * map.num_vertices);
	for (size_t i = 0; i < map.num_vertices; i++)
	{
		remapped_vertices[i] = (vec2){
			.x = (__max(map.min.x, __min(map.vertices[i].x, map.max.x)) - map.min.x) * (out_max.x - out_min.x) / (map.max.x - map.min.x) + out_min.x,
			.y = HEIGHT - (__max(map.min.y, __min(map.vertices[i].y, map.max.y)) - map.min.y) * (out_max.y - out_min.y) / (map.max.y - map.min.y) - out_min.y
		};
	}

	renderer_init(WIDTH, HEIGHT);

	char title[128];
	float angle = 0.0f;
	float last = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		float now = glfwGetTime();
		float delta = now - last;
		last = now;

		angle += 1.0f * delta;

		glfwPollEvents();
		snprintf(title, 128, "Doom1993-Remake | %.0f fps", 1.0f / delta);
		glfwSetWindowTitle(window, title);

		renderer_clear();

		for (size_t i = 0; i < map.num_vertices; i++)
			renderer_draw_point(remapped_vertices[i], 3.0f, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f });

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}