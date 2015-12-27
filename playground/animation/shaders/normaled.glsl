// zozbrazi model s tienovanim
uniform mat4 local_to_world;
uniform mat4 local_to_screen;
uniform mat4 world_to_camera;
uniform mat3 normal_to_camera;
uniform vec3 color = vec3(0.7, 0.7, 0.7);

uniform sampler2D diff_tex;
uniform sampler2D norm_tex;

struct directional_light
{
	vec3 direction;
	float intensity;
	vec3 color;
};

struct material_property
{
	vec3 ambient;
	float shininess;
	float intensity;
};

uniform directional_light light = directional_light(normalize(vec3(1)), 1.0, vec3(1));
uniform material_property material = material_property(vec3(0.2), 64.0, 1.0);

#ifdef _VERTEX_

layout(location=0) in vec3 position;
layout(location=1) in vec2 texcoord;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 tangent;

out vec3 l;  // light direction
out vec3 v;  // view direction
out vec2 uv;

void main() 
{
	mat4 local_to_camera = world_to_camera * local_to_world;
	vec3 n = normal_to_camera * normal;
	vec3 t = normal_to_camera * tangent;
	vec3 b = cross(n,t);
	mat3 T_tbn = mat3(t,b,n);
	vec4 p = local_to_camera * vec4(position, 1);
	
	l = mat3(world_to_camera) * light.direction * T_tbn;
	v = normalize(-p.xyz * T_tbn);
	uv = texcoord;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif

#ifdef _FRAGMENT_

in vec3 l;
in vec3 v;
in vec2 uv;

out vec4 fcolor;

void main()
{
	vec3 n = texture(norm_tex, uv).xyz * 2.0 - 1.0;

	// diffuse
	float diff = max(dot(n,l), 0) * light.intensity;
	
	// specular light
	vec3 r = normalize(-reflect(l,n));
	float spec = pow(max(dot(r,v), 0), material.shininess) * material.intensity * light.intensity;
	
	vec4 texel = texture(diff_tex, uv);
	fcolor = vec4((material.ambient + (diff+spec)*light.color) * texel.rgb, texel.a);
}

#endif
