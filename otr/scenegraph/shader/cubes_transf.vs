layout (location = 0) in vec4 position;
layout (location = 3) in vec4 color;

uniform mat4 local_to_screen;  // MVP

out VS_OUT {
	vec4 color;
} vs_out;

void main()
{
	vs_out.color = color;
	gl_Position = local_to_screen * position;
}
