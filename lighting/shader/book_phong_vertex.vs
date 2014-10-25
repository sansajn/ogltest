// the gouraud shading vertex shader
#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

layout (std140) uniform constants  // uniform bufer object
{
	mat4 mv_matrix;
	mat4 view_matrix;
	mat4 proj_matrix;
};

// light and material properties
uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);
uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 128.0;
uniform vec3 ambient = vec3(0.1, 0.1, 0.1);

out VS_OUT 
{
	vec3 color;
} vs_out;

void main()
{
	vec4 P = mv_matrix*position;  // view-space coordinate
	vec3 N = mat3(mv_matrix)*normal;  // normal in view space
	vec3 L = light_pos - P.xyz;  // view-space light vector
	vec3 V = -P.xyz;  // view vector
	
	N = normalize(N);
	L = normalize(L);
	V = normalize(V);

	vec3 R = reflect(-L, N);  // reflect -L around the plane defined by N

	vec3 diffuse = max((dot(N,L), 0.0)*diffuse_albedo;
	vec3 specular = pow(max(dot(R,V), 0.0), specular_power)*specular_albedo;

	vs_out.color = ambient + diffuse + specular;

	gl_Position = proj_matrix * P;
}

