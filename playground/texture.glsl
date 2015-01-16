#ifdef _VERTEX_

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

uniform mat4 MVP;

out VS_OUT {
	vec2 uv;
} vs_out;

void main()
{
	vs_out.uv = uv;
	gl_Position = MVP * vec4(position, 1);
}

#endif  // _VERTEX_


#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
} fs_in;

uniform sampler2D diffuse;

out vec4 fcolor;

void main()
{
	fcolor = texture(diffuse, fs_in.uv);
}

#endif  // _FRAGMENT_
