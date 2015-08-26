#ifdef _VERTEX_
layout(location = 0) in vec3 position;
uniform mat4 local_to_screen;

void main()
{
	gl_Position = local_to_screen * vec4(position, 1);
}
#endif

#ifdef _FRAGMENT_
out vec4 color;

void main()
{
	color = vec4(0.5, 0.5, 0.5, 1);
}
#endif
