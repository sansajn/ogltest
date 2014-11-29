#pragma once

enum class uniform_type
{
	vec1f, vec1d, vec1i, vec1ui, vec1b,
	vec2f, vec2d, vec2i, vec2ui, vec2b,
	vec3f, vec3d, vec3i, vec3ui, vec3b,
	vec4f, vec4d, vec4i, vec4ui, vec4b,
	mat2f, mat2d,
	mat3f, mat3d,
	mat4f, mat4d,
	mat2x3f, mat2x3d,
	mat2x4f, mat2x4d,
	mat3x2f, mat3x2d,
	mat3x4f, mat3x4d,
	mat4x2f, mat4x2d,
	mat4x3f, mat4x3d,
	sampler1d,
	sampler2d,
	sampler1d_array,
	sampler2d_array
};
