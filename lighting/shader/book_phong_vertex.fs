#version 330 core

layout (location = 0) out vec4 color;

in VS_OUT
{
	vec3 color;
} fs_in;

void main()
{
	color = vec4(fs_in.color, 1.0);  // just write incoming color to the framebuffer
}
