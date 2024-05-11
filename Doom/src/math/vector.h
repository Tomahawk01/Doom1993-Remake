#pragma once

typedef union vec2
{
	struct
	{
		float x, y;
	};

	float v[2];
} vec2;

typedef union vec3
{
	struct
	{
		float x, y, z;
	};
	struct
	{
		float r, g, b;
	};

	float v[3];
} vec3;

typedef union vec4
{
	struct
	{
		float x, y, z, w;
	};
	struct
	{
		float r, g, b, a;
	};

	float v[4];
} vec4;

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_scale(vec3 v, float s);

float vec3_length(vec3 v);
vec3 vec3_normalize(vec3 v);

float vec3_dot(vec3 a, vec3 b);
vec3 vec3_cross(vec3 a, vec3 b);
