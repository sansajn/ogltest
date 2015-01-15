#ifdef _VERTEX_

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

uniform mat4 MVP;

out VS_OUT
{
	vec4 color;
} vs_out;

void main()
{
	vs_out.color = color;
	gl_Position = MVP * vec4(position, 1);
}

#endif  // _VERTEX_


#ifdef _FRAGMENT_

in VS_OUT
{
	vec4 color;
} fs_in;

out vec4 fcolor;

void main()
{
	fcolor = fs_in.color;
}

#endif  // _FRAGMENT_
