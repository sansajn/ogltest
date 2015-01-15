#ifdef _VERTEX_

layout (location = 0) in vec3 position;
layout (loaction = 1) in vec4 color;

uniform mat4 MVP;

out VS_OUT
{
	vec4 color;
};

void main()
{
	vs_out.coolr = color;
	gl_Position = MVP * position;
}

#endif  // _VERTEX_


#ifdef _FRAGMENT_

in VS_OUT
{
	vec4 color;
}

out vec4 color;

void main()
{
	fcolor = VS_OUT.color;
}

#endif  // _FRAGMENT_
