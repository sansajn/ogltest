#ifdef _VERTEX_

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 local_to_world;
uniform mat4 local_to_screen;

out VS_OUT {
	vec2 uv;
	vec3 normal;
	vec3 world_pos;
} vs_out;

void main()
{
	vs_out.uv = uv;
	vs_out.normal = normalize(local_to_world * vec4(normal, 0)).xyz;
	vs_out.world_pos = (local_to_world * vec4(position, 0)).xyz;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif  // _VERTEX_


#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
	vec3 normal;
	vec3 world_pos;
} fs_in;

uniform sampler2D diffuse;

uniform vec3 camera_position;

// light
uniform vec3 light_direction = vec3(1,1,1);
uniform vec3 light_color = vec3(1,1,1);
uniform float light_intensity = 0.4;

// material
uniform float material_specular_intensity = 1.0;
uniform float material_specular_power = 8.0;

out vec4 fcolor;

vec4 calc_directional_light()
{
	// diffuse
	vec3 n = fs_in.normal;
	vec3 l = normalize(light_direction);
	float i_diff = dot(l, n);
	
	vec3 diff_color = vec3(0, 0, 0);
	vec3 spec_color = vec3(0, 0, 0);
	
	if (i_diff > 0)
	{
		diff_color = light_color * i_diff;
		
		// specular
		vec3 v = normalize(camera_position - world_pos);
		vec3 h = normalize(l+v);
		
		float i_spec = pow(dot(h, v), material_specular_power);
		
		if (i_spec > 0)
			spec_color = light_color * material_specular_intensity * i_spec;
	}
	
	return vec4(diff_color + spec_color, 1);
}

void main()
{
	vec4 texel = texture(diffuse, fs_in.uv);
	vec4 dir_light = calc_directional_light();
	fcolor = texel*vec4(0.2, 0.2, 0.2, 1) + texel*dir_light;
}

#endif  // _FRAGMENT_
