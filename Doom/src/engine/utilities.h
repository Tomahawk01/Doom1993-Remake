#pragma once

#include "math/matrix.h"
#include "math/vector.h"
#include "map.h"

void insert_stencil_quad(mat4 transformation);
sector* map_get_sector(vec2 position);
