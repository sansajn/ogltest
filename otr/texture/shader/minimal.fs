# version 330
uniform sampler2D sampler;
uniform vec2 scale;
layout (location = 0) out vec4 data;

void main()
{
	data = texture(sampler, gl_FragCoord.xy * scale).rrrr;
}

