# version 330
uniform sampler2DArray sampler;
uniform vec2 scale;
uniform int layer;
layout (location = 0) out vec4 data;

void main()
{
	data = texture(sampler, vec3(gl_FragCoord.xy * scale, layer)).rrrr;
}

