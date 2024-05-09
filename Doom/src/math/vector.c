#include "math/vector.h"

#include <math.h>

float vec3_length(vec3 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3 vec3_normalize(vec3 v)
{
	float l = vec3_length(v);
	return (vec3) { v.x / l, v.y / l, v.z / l };
}
