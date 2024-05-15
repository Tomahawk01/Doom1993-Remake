#include "math/vector.h"

#include <math.h>

vec2 vec2_add(vec2 a, vec2 b)
{
	return (vec2) { a.x + b.x, a.y + b.y };
}

vec2 vec2_sub(vec2 a, vec2 b)
{
	return (vec2) { a.x - b.x, a.y - b.y };
}

vec3 vec3_add(vec3 a, vec3 b)
{
	return (vec3) { a.x + b.x, a.y + b.y, a.z + b.z };
}

vec3 vec3_sub(vec3 a, vec3 b)
{
	return (vec3) { a.x - b.x, a.y - b.y, a.z - b.z };
}

vec3 vec3_scale(vec3 v, float s)
{
	return (vec3) { v.x * s, v.y * s, v.z * s };
}

float vec3_length(vec3 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3 vec3_normalize(vec3 v)
{
	float l = vec3_length(v);
	return (vec3) { v.x / l, v.y / l, v.z / l };
}

float vec3_dot(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 vec3_cross(vec3 a, vec3 b)
{
	return (vec3) { 
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}
