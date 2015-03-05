#ifdef _VERTEX_
layout(location = 0) in vec3 position;
uniform mat4 transform = mat4(1);
out vec2 st;

void main()
{
	st = position.xy * 0.5 + 0.5;
	gl_Position = transform * vec4(position, 1);
}
#endif

#ifdef _FRAGMENT_
in vec2 st;
uniform sampler2DArray s;
uniform int layer = 0;
out vec4 color;

void main()
{
	color = texture(s, vec3(st, layer));
}
#endif
