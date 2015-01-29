#ifdef _VERTEX_

layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;

uniform mat4 local_to_screen;

out VS_OUT {
	vec2 uv;
} vs_out;

void main()
{
	vs_out.uv = uv;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif // _VERTEX_

#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
} fs_in;

uniform sampler2D diffuse;

// material
uniform vec3 ambient = vec3(0.1, 0.1, 0.1);

out vec4 fcolor;

void main()
{
	fcolor = texture(diffuse, fs_in.uv) * vec4(ambient,1);
}

#endif // _FRAGMENT_
