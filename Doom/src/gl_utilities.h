#pragma once
#include "glad/glad.h"

#include <stddef.h>
#include <stdint.h>

GLuint compile_shader(GLenum type, const char* src);
GLuint link_shader(size_t num_shaders, ...);

GLuint generate_texture(uint16_t width, uint16_t height, uint8_t* data);
