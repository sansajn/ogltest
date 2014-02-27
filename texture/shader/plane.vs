#version 430

layout(location = 0) in vec3 position;
uniform mat4 MVP;  // model-view-projection matrix
out vec2 tex_coord;

void main()
{
	tex_coord = position.xz;
	gl_Position = MVP * vec4(position, 1.0);
}

