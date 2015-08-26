// prallax displacement mapping

#ifdef _VERTEX_

layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 tangent;

uniform mat4 camera_to_screen;
uniform mat4 world_to_camera;
uniform mat4 local_to_world;
uniform mat3 normal_to_camera;

out VS_OUT {
	vec3 v;  // view-direction (not normalized) in tangent space
	vec2 uv;
} vs_out;

void main()
{
	vec3 n = normalize(normal_to_camera * normal);
	vec3 t = normalize(normal_to_camera * tangent);
	vec3 b = cross(n,t);
	mat3 tbn = mat3(t,b,n);

	mat4 local_to_camera = world_to_camera * local_to_world;
	mat4 local_to_screen = camera_to_screen * local_to_camera;

	vec4 p = local_to_camera * vec4(position,1);
	vs_out.v = -p.xyz * tbn;

	vs_out.uv = uv;
	gl_Position = local_to_screen * vec4(position,1);
}

#endif

#ifdef _FRAGMENT_

in VS_OUT {
	vec3 v;  // view-direction (not normalized) in tangent space
	vec2 uv;
} fs_in;

uniform sampler2D diffuse;
uniform sampler2D heightmap;
uniform vec2 parallax_scale_bias = vec2(0.04, -0.03);

out vec4 fcolor;

void main()
{
	vec3 v = normalize(fs_in.v);
	float height = texture(heightmap, fs_in.uv).r;
	height = height * parallax_scale_bias.x + parallax_scale_bias.y;
	vec2 uv = fs_in.uv + (height * v.xy);
	vec4 texel = texture(diffuse, uv);
	fcolor = texel;
}

#endif
