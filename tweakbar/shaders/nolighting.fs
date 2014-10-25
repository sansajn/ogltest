#version 330 core

in vec4 Color;  // interpolated between vertices

out vec4 FragColor;  // color resultfrom this fragment

void main()
{
	FragColor = Color;
}
