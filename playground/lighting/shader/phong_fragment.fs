#version 330

out vec4 color;

in VS_OUT
{
	vec3 n;
	vec3 l;
	vec3 v;
} fs_in;

// material properties
uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 128.0;
uniform vec3 ambient = vec3(0.1, 0.1, 0.1);

void main()
{
	vec3 n = normalize(fs_in.n);
	vec3 l = normalize(fs_in.l);
	vec3 v = normalize(fs_in.v);
	vec3 r = reflect(-l, n);
	vec3 diffuse = max(dot(n,l), 0.0)*diffuse_albedo;
	vec3 specular = pow(max(dot(r,v), 0.0), specular_power)*specular_albedo;
	color = vec4(ambient + diffuse + specular, 1.0);
}
