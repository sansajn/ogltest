#version 330 core

out vec4 color;

in VS_OUT  // input from vertex shader
{
	vec3 color;
} fs_in;

void main()
{
	color = vec4(fs_in.color, 1.0);
}
