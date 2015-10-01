// zobrazi model v jednej farbe

uniform mat4 local_to_screen;
uniform mat3 normal_to_world;
uniform vec3 color = vec3(0.5, 0.5, 0.5);

#ifdef _VERTEX_

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

out VS_OUT {
	vec2 uv;
	vec3 normal;
} vs_out;

void main()
{
	vs_out.uv = uv;
	vs_out.normal = normal_to_world * normal;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
	vec3 normal;
} fs_in;

out vec4 fcolor;

vec3 light_direction = normalize(vec3(1,1,1));

void main()
{
	// do simple lighting based on normals
	float light_intensity = clamp(dot(normalize(fs_in.normal), light_direction), 0.4, 1);

	if (!gl_FrontFacing)
		fcolor = vec4(1, 0, 0, 1);
	else
		fcolor = vec4(light_intensity * color, 1);
}

#endif  // _FRAGMENT_
