// parallax displacement mapping

#ifdef _VERTEX_

struct directional_light
{
	vec3 direction;
	vec3 color;
	float intensity;
};

layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;

uniform mat4 camera_to_screen;
uniform mat4 world_to_camera;
uniform mat4 local_to_world;
uniform mat3 normal_to_camera;

uniform directional_light light;

out VS_OUT {
	vec3 v;  // view-direction in camera space (not normalized)
	vec3 l;  // light-direction in camera space (not normalized)
	vec3 n;  // normal-direction in camera space (not normalized)
	vec2 uv;
} vs_out;

void main()
{
	mat4 local_to_camera = world_to_camera * local_to_world;
	mat4 local_to_screen = camera_to_screen * local_to_camera;

	vs_out.n = normalize(normal_to_camera * normal);

	vec4 p = local_to_camera * vec4(position,1);
	vs_out.v = -p.xyz;  // view in camera space
	
	vec3 l = vec3(world_to_camera * vec4(-light.direction,0));
	vs_out.l = l;  // light in camera space
	
	vs_out.uv = uv;
	gl_Position = local_to_screen * vec4(position,1);
}

#endif

#ifdef _FRAGMENT_

struct directional_light
{
	vec3 direction;
	vec3 color;
	float intensity;
};

struct material_prop {
	vec3 ambient;
	float shininess;
	float intensity;  // shininess intensity
};

in VS_OUT {
	vec3 v;  // view-direction in camera space (not normalized)
	vec3 l;  // light-direction in camera space (not normalized)
	vec3 n;  // normal-direction in camera space (not normalized)
	vec2 uv;
} fs_in;

uniform sampler2D diffuse;
uniform directional_light light;
uniform material_prop material;

out vec4 fcolor;

void main()
{
	vec3 v = normalize(fs_in.v);
	vec3 l = normalize(fs_in.l);
	vec3 n = normalize(fs_in.n);
	vec3 r = normalize(-reflect(l,n));
	
	vec3 amb = material.ambient;
	float diff = max(dot(n,l), 0.0) * light.intensity;
	float spec = pow(max(dot(r,v), 0.0), material.shininess) * material.intensity * light.intensity;
		
	vec4 texel = texture(diffuse, fs_in.uv);
	
	fcolor = vec4(amb*light.color + (diff+spec) * light.color, 1) * texel;
}

#endif
