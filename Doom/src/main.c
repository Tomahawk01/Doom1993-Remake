#include "engine.h"
#include "renderer.h"
#include "wad_loader.h"
#include "input.h"

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

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Doom1993-Remake", NULL, NULL);
	glfwMakeContextCurrent(window);

	// V-Sync
	glfwSwapInterval(0);

	if (!gladLoadGLLoader(glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to initialize Glad\n");
		return -1;
	}

	printf("OpenGL Info:\n");
	printf("	Vendor: %s\n", glGetString(GL_VENDOR));
	printf("	Renderer: %s\n", glGetString(GL_RENDERER));
	printf("	Version: %s\n", glGetString(GL_VERSION));

	// Input handling
	input_init(window);
	glfwSetKeyCallback(window, input_key_callback);
	glfwSetMouseButtonCallback(window, input_mouse_button_callback);
	glfwSetCursorPosCallback(window, input_mouse_position_callback);

	wad wad;
	if (wad_load_from_file("res/doom.wad", &wad) != 0)
	{
		printf("Failed to load WAD file\n");
		return -1;
	}

	renderer_init(WIDTH, HEIGHT);
	engine_init(&wad, "E1M1");

	char title[128];
	float last = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		float now = glfwGetTime();
		float delta = now - last;
		last = now;

		engine_update(delta);

		glfwPollEvents();
		snprintf(title, 128, "Doom1993-Remake | %.0f fps", 1.0f / delta);
		glfwSetWindowTitle(window, title);

		renderer_clear();
		engine_render();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}