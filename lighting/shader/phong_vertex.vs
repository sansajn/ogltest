// the gouraud shading vertex shader
#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
//layout (location = 3) in vec4 color;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

// light and material properties
uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);
uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 128;
uniform vec3 ambient = vec3(0.1, 0.1, 0.1);

out VS_OUT
{
	vec3 color;
} vs_out;

void main()
{
	vec4 p = mv_matrix*position;  // point in view-space
	vec3 n = mat3(mv_matrix)*normal;  // normal in view-space
	
	/* light vector in view-space (svetlo je v nekonecne, preto je od kameri
	vzdialene spale rovnako) */
	vec3 l = light_pos - p.xyz;

	vec3 v = -p.xyz;

	n = normalize(n);
	l = normalize(l);
	v = normalize(v);

	vec3 r = reflect(-l, n);  // reflect -l around the plane defined by n

	vec3 diffuse = max(dot(n,l), 0.0)*diffuse_albedo;
	vec3 specular = pow(max(dot(r,v), 0.0), specular_power)*specular_albedo;
	
	vs_out.color = diffuse + specular + ambient;
	gl_Position = proj_matrix * p;
}
