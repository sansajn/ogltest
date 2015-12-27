// zozbrazi farebny model
uniform mat4 local_to_screen;

#ifdef _VERTEX_

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out vec3 vcolor;

void main()
{
	vcolor = color;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif

#ifdef _FRAGMENT_

in vec3 vcolor;
out vec4 fcolor;

void main()
{
	fcolor = vec4(vcolor, 1);
}

#endif

