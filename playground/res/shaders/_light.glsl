#ifdef _VERTEX_

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 local_to_camera;
uniform mat4 local_to_screen;

uniform vec3 light_pos = vec3(0, 100, -100);

out VS_OUT {
	vec3 n;
	vec3 l;
	vec3 v;
	vec2 uv;
} vs_out;

void main()
{
	vec4 p = local_to_camera * vec4(position, 1);
	vs_out.n = mat3(local_to_camera) * normal;
	vs_out.l = light_pos - p.xyz;
	vs_out.v = -p.xyz;
	vs_out.uv = uv;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif  // _VERTEX_


#ifdef _FRAGMENT_

in VS_OUT {
	vec3 n;
	vec3 l;
	vec3 v;
	vec2 uv;
} fs_in;

uniform sampler2D diffuse;

// light
uniform vec3 light_color = vec3(1,1,1);
uniform float light_intensity = 0.4;

// material
uniform float material_specular_intensity = 1.0;
uniform float material_specular_power = 8.0;

out vec4 fcolor;

vec4 calc_directional_light()
{
	vec3 n = normalize(fs_in.n);
	vec3 l = normalize(fs_in.l);
	vec3 v = normalize(fs_in.v);
	vec3 r = reflect(-l, n);
	vec3 diffuse = max(dot(n, l), 0.0) * light_intensity * light_color;
	vec3 specular = pow(max(dot(r,v), 0.0), material_specular_power) * material_specular_intensity * light_color;
	return vec4(diffuse+specular, 1.0);
}

void main()
{
	vec4 texel = texture(diffuse, fs_in.uv);
	vec4 dir_light = calc_directional_light();
	fcolor = texel*vec4(0.2, 0.2, 0.2, 1) + texel*dir_light;
}

#endif  // _FRAGMENT_
