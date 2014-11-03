#version 330

layout(location = 0) in vec3 position;
layout(location = 3) in vec4 color;

uniform mat4 mvp;

out vec4 fcolor;

void main()
{
	fcolor = color;
	gl_Position = mvp * vec4(position, 1.0);
}
