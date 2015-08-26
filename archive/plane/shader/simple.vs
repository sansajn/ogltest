#version 430

layout(location = 0) in vec3 vertex_position;

uniform mat4 mvp;  // model-view-projection matrix
uniform vec4 model_color;

void main()
{
	gl_Position = mvp * vec4(vertex_position, 1.0);
}

