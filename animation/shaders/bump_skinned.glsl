// zozbrazi model s phong osvetlenim a parallax mapovanim
uniform mat4 local_to_world;
uniform mat4 local_to_screen;
uniform mat4 world_to_camera;
uniform mat3 normal_to_camera;

uniform sampler2D diff_tex;
uniform sampler2D norm_tex;
uniform sampler2D height_tex;

const int MAX_JOINTS = 100;
uniform mat4 skeleton[MAX_JOINTS];  // kostra

uniform vec2 parallax_scale_bias = vec2(0.04, -0.03);

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
layout(location=4) in ivec4 joints;
layout(location=5) in vec4 weights;

out vec3 l;  // light direction
out vec3 v;  // view direction
out vec2 uv;

void main() 
{
	mat4 T_skin =
		skeleton[joints.x] * weights.x +
		skeleton[joints.y] * weights.y +
		skeleton[joints.z] * weights.z +
		skeleton[joints.w] * weights.w;

	mat4 local_to_camera = world_to_camera * local_to_world;
	vec3 n = normal_to_camera * vec3(T_skin * vec4(normal, 0));
	vec3 t = normal_to_camera * vec3(T_skin * vec4(tangent, 0));
	vec3 b = cross(n,t);
	mat3 T_tbn = mat3(t,b,n);
	vec4 p_skinned = T_skin * vec4(position, 1);
	vec4 p = local_to_camera * p_skinned;
	
	l = mat3(world_to_camera) * light.direction * T_tbn;
	v = normalize(-p.xyz * T_tbn);
	uv = texcoord;
	
	gl_Position = local_to_screen * p_skinned;
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
	
	float h = texture(height_tex, uv).r * parallax_scale_bias.x + parallax_scale_bias.y;
	vec2 uv_ = uv + h * v.xy;
	vec4 texel = texture(diff_tex, uv_);
	
	fcolor = vec4((material.ambient + (diff+spec)*light.color) * texel.rgb, texel.a);
}

#endif
