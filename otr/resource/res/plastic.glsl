#ifdef _VERTEX_

layout(location = 1) in vec3 normal;
layout(location = 3) in vec4 color;

out VS_OUT {
	vec4 color;
} vs_out;

void projection();

void main()
{
	projection();
	vs_out.color = color;
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

in VS_OUT {
	vec4 color;
} fs_in;

out vec4 fcolor;

void main()
{
	fcolor = fs_in.color;
}

#endif  // _FRAGMENT_
