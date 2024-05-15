#pragma once
#include "math/vector.h"

#define WORLD_UP ((vec3){0.0f, 1.0f, 0.0f})

typedef struct camera
{
	vec3 position;
	float pitch;
	float yaw;

	vec3 forward;
	vec3 right;
	vec3 up;
} camera;

void camera_update_direction_vectors(camera* c);
