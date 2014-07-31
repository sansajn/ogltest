#version 330

in vec3 position;
out vec2 tex_coord;

void main()
{
	tex_coord = position.xy;
	gl_Position = vec4(position, 1.0);
}

