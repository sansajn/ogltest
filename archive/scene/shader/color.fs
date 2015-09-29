#version 430

layout(location = 0) out vec4 color;  // fragment color

uniform vec4 model_color;

void main()
{
	color = model_color;
}
