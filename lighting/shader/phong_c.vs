/* implementuje phongov osvetlovaci model s ohladom na farbu objektu */
#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec4 color;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);

out VS_OUT 
{
	vec3 n;
	vec3 l;
	vec3 v;
	vec4 color;
} vs_out;

void main()
{
	vec4 p = mv_matrix*position;
	vs_out.n = mat3(mv_matrix)*normal;
	vs_out.l = light_pos - p.xyz;
	vs_out.v = -p.xyz;
	vs_out.color = color;
	gl_Position = proj_matrix*p;
}

