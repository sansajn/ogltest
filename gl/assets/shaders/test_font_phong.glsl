uniform mat4 local_to_screen;
uniform mat4 local_to_camera;

#ifdef _VERTEX_
layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;

out vec3 n;  // normal in camera space (not normalized)

void main()
{
	n = mat3(local_to_camera) * normal;
	gl_Position = local_to_screen * vec4(position, 1);
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_
in vec3 n;  // normal in camera space (not normalized)
out vec4 fcolor;
const vec3 to_light_dir = normalize(vec3(1.0));

void main()
{
	float light = clamp(dot(normalize(n), to_light_dir), 0, 1);
	fcolor = vec4(vec3(0.1, 0.1, 0.1) + light * vec3(0.5, 0.5, 0.5), 1);
}

#endif  // _FRAGMENT_
