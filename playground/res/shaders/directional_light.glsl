#ifdef _VERTEX_

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 M;    // local_to_camera
uniform mat4 MVP;  // local_to_screen

out VS_OUT {
	vec2 uv;
	vec3 normal;
	vec3 world_pos;
} vs_out;

void main()
{
	vs_out.uv = uv;
	vs_out.normal = normalize(M * vec4(normal, 0)).xyz;
	vs_out.world_pos = (M * vec4(position, 0)).xyz;
	gl_Position = MVP * vec4(position, 1);
}

#endif  // _VERTEX_


#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
	vec3 normal;
	vec3 world_pos;
} fs_in;

uniform sampler2D diffuse;

uniform vec3 eye_pos;  // or camera position

uniform vec3 light_direction = vec3(-1,-1,-1);
uniform vec3 light_color = vec3(1,1,1);
uniform float light_intensity = 0.4;

uniform float material_specular_intensity = 1.0;
uniform float material_specular_power = 8.0;

out vec4 fcolor;

vec4 calc_directional_light(vec3 direction, vec3 normal, vec3 world_pos)
{
	float diffuse_factor = dot(normal, -direction);

	vec4 diffuse_color = vec4(0,0,0,0);
	vec4 specular_color = vec4(0,0,0,0);

	if (diffuse_factor > 0)
	{
		diffuse_color = vec4(light_color, 1) * light_intensity * diffuse_factor;

		vec3 eye_dir = normalize(eye_pos - world_pos);
		vec3 half_dir = normalize(eye_dir - direction);
		float specular_factor = dot(half_dir, normal);
		specular_factor = pow(specular_factor, material_specular_power);

		if (specular_factor > 0)
			specular_color = vec4(light_color, 1) * material_specular_intensity * specular_factor;
	}

	return diffuse_color + specular_color;
}

void main()
{
	vec4 directional_light = calc_directional_light(light_direction, fs_in.normal, fs_in.world_pos);
	vec4 texel = texture(diffuse, fs_in.uv);
	fcolor = texel*vec4(0.2, 0.2, 0.2, 1) + texel * directional_light;
}

#endif  // _FRAGMENT_
