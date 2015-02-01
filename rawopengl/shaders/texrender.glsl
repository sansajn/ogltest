#ifdef _VERTEX_

layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;

out VS_OUT {
	vec2 uv;
} vs_out;

void main()
{
	vs_out.uv = uv;
	gl_Position = vec4(position, 1);
}

#endif

#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
} fs_in;

uniform sampler2D tex;

out vec4 fcolor;

void main()
{
	fcolor = texture(tex, fs_in.uv);
}

#endif
