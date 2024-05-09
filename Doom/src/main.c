#include "renderer.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>

#define WIDTH 1280
#define HEIGHT 720

int main(int argc, char** argv)
{
	if (glfwInit() != GLFW_TRUE)
	{
		printf(stderr, "Failed to initialize GLFW\n");
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
		printf(stderr, "Failed to initialize Glad\n");
		return -1;
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
		// FOR TESTING
		{
			renderer_draw_point((vec2) { WIDTH / 2.0f, HEIGHT / 2.0f }, 5.0f, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f });
			renderer_draw_line((vec2) { 800.0f, 200.0f }, (vec2) { WIDTH - 300.0f, HEIGHT - 300.0f }, 4.0f, (vec4) { 0.0f, 1.0f, 0.0f, 1.0f });
			renderer_draw_line((vec2) { 1000.0f, 200.0f }, (vec2) { WIDTH - 500.0f, HEIGHT - 300.0f }, 4.0f, (vec4) { 0.0f, 1.0f, 0.0f, 1.0f });
			renderer_draw_quad((vec2) { 100.0f, 100.0f }, (vec2) { 50.0f, 50.0f }, angle, (vec4) { 0.5f, 0.5f, 1.0f, 1.0f });
		}
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}