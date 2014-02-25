#version 130

in vec4 color;
out vec4 fColor;

void main () {
	// No matter what, color the pixel red!
	fColor = color;
}