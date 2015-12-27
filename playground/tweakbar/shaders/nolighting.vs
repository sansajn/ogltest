#version 330 core

uniform mat4 local_to_screen;  // model-view-projection transformation

layout (location = 0) in vec4 position;
layout (location = 3) in vec4 color;

out vec4 Color;

void main()
{
	Color = color;
	gl_Position = local_to_screen * position;
}
