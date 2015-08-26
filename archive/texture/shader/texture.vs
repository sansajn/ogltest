#version 330

in vec3 position;
uniform mat4 MVP;
out vec2 tex_coord;

void main()
{
	tex_coord = position.xy;
	gl_Position = MVP * vec4(position, 1.0);
}

