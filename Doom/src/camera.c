#include "camera.h"

#include <math.h>

void camera_update_direction_vectors(camera* c)
{
	c->forward = (vec3){
		cosf(c->yaw) * cosf(c->pitch),
		sinf(c->pitch),
		sinf(c->yaw) * cosf(c->pitch)
	};

	c->right = vec3_normalize(vec3_cross(WORLD_UP, c->forward));
	c->up = vec3_normalize(vec3_cross(c->forward, c->right));
}
