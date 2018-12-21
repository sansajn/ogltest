// zobrazi model bez osvetlenia v zakladnej farbe
uniform mat4 local_to_screen;
uniform vec3 color = vec3(0.7, 0.7, 0.7);

#ifdef _VERTEX_

layout(location=0) in vec3 position;

void main()
{
	gl_Position = local_to_screen * vec4(position,1);
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

out vec4 fcolor;

void main()
{
	fcolor = vec4(color, 1);
}

#endif  // _FRAGMENT_
