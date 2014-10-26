#version 330 core

in VS_OUT
{
	vec3 n;
	vec3 l;
	vec3 v;
	vec4 color;
} fs_in;

// material properties
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 128.0;
uniform float ambient = 0.1;

out vec4 fcolor;

void main()
{
	vec3 n = normalize(fs_in.n);
	vec3 l = normalize(fs_in.l);
	vec3 v = normalize(fs_in.v);
	vec3 r = reflect(-l, n);
	vec3 diffuse_albedo = fs_in.color.xyz;
	vec3 diffuse = max(dot(n,l), 0.0)*diffuse_albedo;
	vec3 specular = pow(max(dot(r,v), 0.0), specular_power)*specular_albedo;
	fcolor = vec4(ambient*fs_in.color.xyz + diffuse + specular, 1.0);
}
