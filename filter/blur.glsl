/* Dvojpriechodova implementacia gausovho filtra. */

#ifdef _VERTEX_

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out VS_OUT {
	vec2 uv;
} vs_out;

void main()
{
	vs_out.uv = uv;
	gl_Position = vec4(position,1);
}

#endif

#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
} fs_in;

uniform sampler2D tex;
uniform vec2 direction;  // vec2(dx,0) or vec2(0,dy)
uniform float weights[5];

out vec4 fcolor;

void main()
{
	vec2 uv = fs_in.uv;
	vec4 sum = vec4(0,0,0,0);
	sum += texture(tex, uv + direction*-4) * weights[4];
	sum += texture(tex, uv + direction*-3) * weights[3];
	sum += texture(tex, uv + direction*-2) * weights[2];
	sum += texture(tex, uv - direction) * weights[1];
	sum += texture(tex, uv) * weights[0];
	sum += texture(tex, uv + direction) * weights[1];
	sum += texture(tex, uv + direction*2) * weights[2];
	sum += texture(tex, uv + direction*3) * weights[3];
	sum += texture(tex, uv + direction*4) * weights[4];
	fcolor = sum;
}

#endif
