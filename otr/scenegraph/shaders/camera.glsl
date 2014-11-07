uniform mat4 local_to_screen;

#ifdef _VERTEX_

layout(location = 0) in vec3 position;

void projection()
{
	gl_Position = local_to_screen * vec4(position, 1.0);
}

#endif  // _VERTEX_

