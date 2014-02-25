#version 130

in vec4 s_vPosition;
in vec4 s_vColor;
out vec4 color;
//uniform mat4 s_mM;	// The matrix for the pose of the model
//uniform mat4 s_mV;	// The matrix for the pose of the camera
//uniform mat4 s_mP;	// The perspective matrix

void main () {
	// Look, Ma!  I avoided any matrix multiplication!
	// The value of s_vPosition should be between -1.0 and +1.0 (to be visible on the screen)
	//gl_Position = s_mP*s_mV*s_mM*s_vPosition;
	color = s_vColor;
	gl_Position = s_vPosition;
}