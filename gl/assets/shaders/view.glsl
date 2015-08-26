#ifdef _VERTEX_

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform mat4 transform;

out VS_OUT {
	vec2 uv;
	vec3 normal;
} vs_out;

void main()
{
	vs_out.uv = uv;
	vs_out.normal = normal;
	gl_Position = transform * vec4(position, 1);
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
	vec3 normal;
} fs_in;

uniform sampler2D tex;

out vec4 fcolor;

vec3 light_direction = vec3(1,1,1);

void main()
{
	float light_intensity = clamp(dot(fs_in.normal, light_direction), 0.2, 1);
	fcolor = light_intensity * texture(tex, fs_in.uv);
}

#endif  // _FRAGMENT_
