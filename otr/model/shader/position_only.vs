#version 330

layout(location = 0) in vec3 position;

uniform mat4 mvp;

out vec4 fcolor;

void main()
{
	fcolor = vec4(8.0, 0.0, 0.0, 1.0);
	gl_Position = mvp * vec4(position, 1.0);
}
