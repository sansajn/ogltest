#ifdef _VERTEX_
layout(location = 0) in vec3 position;
out vec2 st;

void main()
{
	st = position.xy * 0.5 + 0.5;
	gl_Position = vec4(position, 1);
}
#endif

#ifdef _FRAGMENT_
in vec2 st;
uniform sampler2DArray s;
//uniform sampler2D s;
uniform int texture_idx = 0;
out vec4 color;

void main()
{
	color = texture(s, vec3(st, texture_idx));
//	color = texture(s, st);
}
#endif
