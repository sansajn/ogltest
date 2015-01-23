// light zo super-bible

#ifdef _VERTEX_

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 uv;
layout (location = 2) in vec3 normal;

out VS_OUT {
	vec3 N;
	vec3 L;
	vec3 V;
	vec2 uv;
} vs_out;

uniform mat4 local_to_camera;
uniform mat4 camera_to_screen;

uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);

void main()
{
	vec4 p = local_to_camera * vec4(position, 1);
	vs_out.N = mat3(local_to_camera) * normal;
	vs_out.L = light_pos - p.xyz;
	vs_out.V = -p.xyz;
	gl_Position = camera_to_screen * p;
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

in VS_OUT {
	vec3 N;
	vec3 L;
	vec3 V;
	vec2 uv;
} fs_in;

//uniform sampler2D diffuse;

// material
uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 8.0;

out vec4 color;

void main()
{
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	vec3 V = normalize(fs_in.V);
	vec3 R = reflect(-L,N);
//	vec3 diffuse = max(dot(N,L), 0.0) * diffuse_albedo;
	vec3 diffuse = vec3(0,0,0);
	vec3 specular = pow(max(dot(R,V), 0.0), specular_power) * specular_albedo;
	color = vec4(diffuse + specular, 1.0);
}

#endif  // _FRAGMENT_
