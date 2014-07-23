#version 330

layout (location = 0) in vec4 position;
layout (location = 3) in vec4 color;

uniform mat4 local_to_screen;  // MVP

out vec4 fcolor;

void main()
{
	fcolor = color;
	gl_Position = local_to_screen * position;
}
