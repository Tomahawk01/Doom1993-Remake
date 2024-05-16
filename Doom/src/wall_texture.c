#include "wall_texture.h"

#include <math.h>

GLuint generate_wall_texture_array(const wall_tex* textures, size_t num_textures, vec2* max_coords_array)
{
    vec2 max_size = { 0.f, 0.f };
    vec2 min_size = { INFINITY, INFINITY };
    for (int i = 0; i < num_textures; i++)
    {
        if (max_size.x < textures[i].width) max_size.x = textures[i].width;
        if (max_size.y < textures[i].height) max_size.y = textures[i].height;
        if (min_size.x > textures[i].width) min_size.x = textures[i].width;
        if (min_size.y > textures[i].height) min_size.y = textures[i].height;
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex_id);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8UI, max_size.x, max_size.y, num_textures);

    for (int i = 0; i < num_textures; i++)
    {
        max_coords_array[i] = (vec2){
            textures[i].width / max_size.x,
            textures[i].height / max_size.y
        };

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, textures[i].width, textures[i].height, 1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, textures[i].data);
    }

    return tex_id;
}
