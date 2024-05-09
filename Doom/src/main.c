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

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}