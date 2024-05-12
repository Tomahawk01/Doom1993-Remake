#include "engine.h"
#include "renderer.h"
#include "wad_loader.h"
#include "input.h"
#include "gl_utilities.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 1920
#define HEIGHT 1080

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
	renderer_set_draw_texture(0);
	//engine_init(&wad, "E1M1");

	palette palette;
	wad_read_playpal(&palette, &wad);
	GLuint palette_texture = palette_generate_texture(&palette);
	renderer_set_palette_texture(palette_texture);

	size_t   num_textures;
	patch* patches = wad_read_patches(&num_textures, &wad);
	GLuint* tex = malloc(sizeof(GLuint) * num_textures);
	for (int i = 0; i < num_textures; i++)
		tex[i] = generate_texture(patches[i].width, patches[i].height, patches[i].data);

	size_t index = 0;
	float  time = .5f;

	char title[128];
	float last = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		float now = glfwGetTime();
		float delta = now - last;
		last = now;

		time -= delta;
		if (time <= 0.0f)
		{
			time = 0.5f;
			if (++index >= num_textures)
				index = 0;
		}

		//engine_update(delta);

		glfwPollEvents();
		snprintf(title, 128, "Doom1993-Remake | %.0f fps", 1.0f / delta);
		glfwSetWindowTitle(window, title);

		renderer_clear();
		//engine_render();
		renderer_set_draw_texture(tex[index]);
		renderer_set_texture_index(0);
		renderer_draw_quad((vec2) { WIDTH / 2.0f, HEIGHT / 2.0f }, (vec2) { patches[index].width * 5.0f, patches[index].height * 5.0f }, 0.0f, 0);
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}