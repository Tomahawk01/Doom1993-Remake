#pragma once

#include "glad/glad.h"

#include <stddef.h>

GLuint compile_shader(GLenum type, const char* src);
GLuint link_shader(size_t num_shaders, ...);
