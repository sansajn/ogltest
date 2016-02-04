// zobrazi otexturovany model (bez osvetlenia)
uniform mat4 local_to_screen;
uniform sampler2D s;  // diffuse texture sampler

#ifdef _VERTEX_
layout(location=0) in vec3 position;
layout(location=1) in vec2 texcoord;
out vec2 uv;

void main()
{
	uv = texcoord;
	gl_Position = local_to_screen * vec4(position,1);
}

#endif

#ifdef _FRAGMENT_
in vec2 uv;
out vec4 fcolor;

void main() 
{
	fcolor = texture(s, uv);
}

#endif