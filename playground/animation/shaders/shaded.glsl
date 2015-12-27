// zozbrazi model s tienovanim
uniform mat4 local_to_screen;
uniform mat3 normal_to_world;
uniform vec3 color = vec3(0.7, 0.7, 0.7);
uniform vec3 light_dir = normalize(vec3(1,1,1));

#ifdef _VERTEX_

layout(location=0) in vec3 position;
layout(location=2) in vec3 normal;

out vec3 n;

void main() 
{
	n = normal_to_world * normal;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif

#ifdef _FRAGMENT_

in vec3 n;
out vec4 fcolor;

void main() 
{
	fcolor = vec4(max(dot(n, light_dir), 0.2) * color, 1);
}

#endif
