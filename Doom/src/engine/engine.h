#pragma once
#include "wad_loader.h"

void engine_init(wad* wad, const char* mapname);
void engine_update(float dt);
void engine_render();
