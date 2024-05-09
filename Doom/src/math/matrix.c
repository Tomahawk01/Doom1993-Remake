#include "math/matrix.h"

#include <math.h>

mat4 mat4_identity()
{
	mat4 mat = { 0 };
	mat.a1 = 1.0f;
	mat.b2 = 1.0f;
	mat.c3 = 1.0f;
	mat.d4 = 1.0f;

	return mat;
}

mat4 mat4_mult(mat4 m1, mat4 m2)
{
	mat4 result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
		}
	}

	return result;
}

mat4 mat4_translate(vec3 translation)
{
	mat4 mat = mat4_identity();
	mat.d1 = translation.x;
	mat.d2 = translation.y;
	mat.d3 = translation.z;

	return mat;
}

mat4 mat4_scale(vec3 scale)
{
	mat4 mat = { 0 };
	mat.a1 = scale.x;
	mat.b2 = scale.y;
	mat.c3 = scale.z;
	mat.d4 = 1.0f;

	return mat;
}

mat4 mat4_rotate(vec3 axis, float angle)
{
	float c = cosf(angle);
	float s = sinf(angle);
	float t = 1.0f - c;

	vec3 axis_normalized = vec3_normalize(axis);
	float x = axis_normalized.x;
	float y = axis_normalized.y;
	float z = axis_normalized.z;

	mat4 result = {
		.a1 = t * x * x + c,
		.a2 = t * x * y - s * z,
		.a3 = t * x * z + s * y,
		.a4 = 0.0f,
		.b1 = t * x * y + s * z,
		.b2 = t * y * y + c,
		.b3 = t * y * z - s * x,
		.b4 = 0.0f,
		.c1 = t * x * z - s * y,
		.c2 = t * y * z + s * x,
		.c3 = t * z * z + c,
		.c4 = 0.0f,
		.d1 = 0.0f,
		.d2 = 0.0f,
		.d3 = 0.0f,
		.d4 = 1.0f,
	};

	return result;
}

mat4 mat4_ortho(float left, float right, float bottom, float top, float nearClip, float farClip)
{
	mat4 mat = { 0 };
	mat.a1 = 2.0f / (right - left);
	mat.b2 = 2.0f / (top - bottom);
	mat.c3 = -2.0f / (farClip - nearClip);
	mat.d1 = -(right + left) / (right - left);
	mat.d2 = -(top + bottom) / (top - bottom);
	mat.d3 = -(farClip + nearClip) / (farClip - nearClip);
	mat.d4 = 1.0f;

	return mat;
}