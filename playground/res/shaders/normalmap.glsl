#ifdef _VERTEX_

layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 tangent;

uniform mat4 local_to_world;
uniform mat4 local_to_screen;

out VS_OUT {
	mat3 tbn;  // tangent, bitangent and normal
	vec3 world_pos;
	vec2 uv;
} vs_out;

mat3 calc_tbn(vec3 normal, vec3 tangent, mat3 rotation)
{
	vec3 n = rotation * normal;
	vec3 t = rotation * tangent;
	t = normalize(t - dot(t,n)*n);
	vec3 b = cross(t,n);
	return mat3(t, b, n);
}

void main()
{
	vec4 p = local_to_world * vec4(position, 1);
	vs_out.tbn = calc_tbn(normal, tangent, mat3(local_to_world));
	vs_out.world_pos = (local_to_world * vec4(position, 1)).xyz;
	vs_out.uv = uv;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif // _VERTEX_

#ifdef _FRAGMENT_

struct directional_light
{
	vec3 color;
	float intensity;
	vec3 direction;
};

in VS_OUT {
	mat3 tbn;
	vec3 world_pos;
	vec2 uv;
} fs_in;

uniform vec3 camera_pos;

uniform sampler2D diffuse;
uniform sampler2D normalmap;

uniform directional_light light;

// material
uniform float spec_intensity = 0.7;
uniform float spec_power = 128.0;

out vec4 fcolor;

vec4 calc_directional_light(directional_light light, vec3 normal, vec3 view,
	float spec_intensity, float spec_power)
{
	vec3 l = light.direction;
	vec3 n = normal;
	vec3 r = reflect(-l, n);
	vec3 v = view;
	float diffuse = max(dot(n,l), 0.0) * light.intensity;
	float specular = pow(max(dot(r,v), 0.0), spec_power) * spec_intensity;
	return vec4((diffuse + specular) * light.color, 1.0);
}

void main()
{
	vec3 n = normalize(fs_in.tbn * (255.0/128.0 * texture(normalmap, fs_in.uv).xyz - 1.0));
	vec3 v = normalize(camera_pos - fs_in.world_pos);
	vec4 dirlight = calc_directional_light(light, n, v, spec_intensity, spec_power);
	vec4 texel = texture(diffuse, fs_in.uv);
	fcolor = texel*dirlight;
}

#endif // _FRAGMENT_
