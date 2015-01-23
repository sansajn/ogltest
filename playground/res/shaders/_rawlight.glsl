#ifdef _VERTEX_

layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;

uniform mat4 local_to_camera;
uniform mat4 camera_to_screen;
uniform vec3 light_pos = vec3(0, 100.0, 100.0);

out VS_OUT {
	vec3 n;
	vec3 l;
	vec3 v;
} vs_out;

void main() 
{
	vec4 p = local_to_camera * vec4(position, 1.0);
	vs_out.n = mat3(local_to_camera) * normal;
	vs_out.l = light_pos - p.xyz;
	vs_out.v = -p.xyz;
	gl_Position = camera_to_screen*p;
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

in VS_OUT {
	vec3 n;
	vec3 l;
	vec3 v;
} fs_in;

uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 16.0;
uniform vec3 ambient = vec3(0.1);

out vec4 fcolor;

void main() 
{
	vec3 n = normalize(fs_in.n);
	vec3 l = normalize(fs_in.l);
	vec3 v = normalize(fs_in.v);
	vec3 r = reflect(-l, n);
	vec3 diffuse = max(dot(n,l), 0.0)*diffuse_albedo;
	vec3 specular = pow(max(dot(r,v), 0.0), specular_power)*specular_albedo;
	fcolor = vec4(ambient + diffuse + specular, 1.0);
//	fcolor = vec4(ambient + specular, 1.0);
}

#endif  // _FRAGMENT_
