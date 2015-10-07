// vyrenderuje texturu
uniform mat4 local_to_screen;
uniform sampler2D diff_tex;

#ifdef _VERTEX_

layout(location=0) in vec3 position;
layout(location=1) in vec2 texcoord;

out vec2 uv;

void main()
{
	uv = texcoord;
	gl_Position = local_to_screen * vec4(position,1);
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

in vec2 uv;
out vec4 fcolor;

void main()
{
	fcolor = texture(diff_tex, uv);
}

#endif  // _FRAGMENT_

