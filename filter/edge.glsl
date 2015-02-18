/* Implemenuje detekciu hran pomocou sobelovho hranoveho detektora

      |-1  0  1|         |-1 -2 -1|
S_x = |-2  0  2|,  S_y = | 0  0  0| 
      |-1  0  1|         | 1  2  1|

g = sqrt(s_x^2 + s_y^2)

kde s_x a s_y su aplykacie matic S_x a S_y na texturu. */

#ifdef _VERTEX_

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out VS_OUT {
	vec2 uv;
} vs_out;

void main()
{
	vs_out.uv = uv;
	gl_Position = vec4(position, 1);
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
} fs_in;

uniform sampler2D tex;
uniform float edge_threshold = 0.5;  // squared threshold value
uniform float dx;  // 1/width
uniform float dy;  // 1/height

out vec4 fcolor;

float luma(vec3 color)  // brightness of a rgb values
{
	return 0.2126*color.r + 0.7152*color.g + 0.0722*color.b;
}

void main()
{
	vec2 uv = fs_in.uv;

	float s00 = luma(texture(tex, uv + vec2(-dx, dy)).rgb);
	float s10 = luma(texture(tex, uv + vec2(-dx, 0.0)).rgb);
	float s20 = luma(texture(tex, uv + vec2(-dx, -dy)).rgb);
	float s01 = luma(texture(tex, uv + vec2(0.0, dy)).rgb);
	float s21 = luma(texture(tex, uv + vec2(0.0, -dy)).rgb);
	float s02 = luma(texture(tex, uv + vec2(dx, dy)).rgb);
	float s12 = luma(texture(tex, uv + vec2(dx, 0.0)).rgb);
	float s22 = luma(texture(tex, uv + vec2(dx, -dy)).rgb);
	
	float sx = s00 + 2*s10 + s20 - (s02 + 2*s12 + s22);
	float sy = s00 + 2*s01 + s02 - (s20 + 2*s21 + s22);

	float dist = sx*sx + sy*sy;

	if (dist > edge_threshold)
		fcolor = vec4(1.0);
	else
		fcolor = vec4(0.0, 0.0, 0.0, 1.0);
}

#endif // _FRAGMENT_
