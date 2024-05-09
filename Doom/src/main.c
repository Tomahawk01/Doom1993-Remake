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
	float last = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		float now = glfwGetTime();
		float delta = now - last;
		last = now;

		snprintf(title, 128, "Doom1993-Remake | %.0f fps", 1.0f / delta);
		glfwSetWindowTitle(window, title);

		glfwPollEvents();
		renderer_clear();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}