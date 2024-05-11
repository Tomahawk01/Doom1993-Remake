#pragma once
#include "math/vector.h"

typedef union mat4
{
	struct
	{
		float a1, a2, a3, a4;
		float b1, b2, b3, b4;
		float c1, c2, c3, c4;
		float d1, d2, d3, d4;
	};

	float m[4][4];
	float v[16];
} mat4;

mat4 mat4_identity();
mat4 mat4_mult(mat4 m1, mat4 m2);

mat4 mat4_translate(vec3 translation);
mat4 mat4_scale(vec3 scale);
mat4 mat4_rotate(vec3 axis, float angle);

mat4 mat4_look_at(vec3 eye, vec3 target, vec3 up);
mat4 mat4_perspective(float fov, float aspect, float nearClip, float farClip);
mat4 mat4_ortho(float left, float right, float bottom, float top, float nearClip, float farClip);
