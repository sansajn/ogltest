/* gles2 skinning using matrix-pallete (OpenGL ES 2.0 Programming Guide, exmple 8-6)
(pre vypocet normal predpoklada, ze transformacne matice su ortonormalne). */

#ifdef _VERTEX_

#define NUM_MATRICES 32

uniform mat4 local_to_screen;
uniform mat4 world_to_camera;
uniform mat3 normal_to_camera;
uniform vec3 light_direction;

// store 32 4x3 matrices, each matrix in row-major order (3x vec4)
uniform vec4 matrix_palette[NUM_MATRICES * 3];

attribute vec3 position;
attribute vec3 normal;
attribute vec4 weights;  // matrix weights (4 entries / vertex)
attribute vec4 indices;  // matrix palette indices (4 entries / vertex)

varying vec3 n;
varying vec3 l;

void skin_position(in vec3 position, float wt, int idx, out vec3 skinned_position)
{
	vec3 tmp;
	vec4 p = vec4(position, 1);
	tmp.x = dot(p, matrix_palette[idx]);
	tmp.y = dot(p, matrix_palette[idx+1]);
	tmp.z = dot(p, matrix_palette[idx+2]);
//	tmp.w = position.w;
	skinned_position += wt * tmp;
}

void skin_normal(in vec3 norm, float wt, int idx, out vec3 skinned_normal)
{
	vec3 tmp;
	tmp.x = dot(norm, matrix_palette[idx].xyz);
	tmp.y = dot(norm, matrix_palette[idx+1].xyz);
	tmp.z = dot(norm, matrix_palette[idx+2].xyz);
	skinned_normal += wt * tmp;
}

void do_skinning(in vec3 position, in vec3 normal, out vec3 skinned_position, out vec3 skinned_normal)
{
	skinned_position = vec3(0.0);
	skinned_normal = vec3(0.0);
	
	/* transform position and normal to eye/camera/view space using matrix palette with 
	four matrices used to transform a vertex */
	
	float wt = weights[0];
	int idx = min(int(indices[0]), NUM_MATRICES-1) * 3;
	skin_position(position, wt, idx, skinned_position);
	skin_normal(normal, wt, idx, skinned_normal);
	
	wt = weights[1];
	idx = min(int(indices[1]), NUM_MATRICES-1) * 3;
	skin_position(position, wt, idx, skinned_position);
	skin_normal(normal, wt, idx, skinned_normal);
	
	wt = weights[2];
	idx = min(int(indices[2]), NUM_MATRICES-1) * 3;
	skin_position(position, wt, idx, skinned_position);
	skin_normal(normal, wt, idx, skinned_normal);
	
	wt = weights[3];
	idx = min(int(indices[3]), NUM_MATRICES-1) * 3;
	skin_position(position, wt, idx, skinned_position);
	skin_normal(normal, wt, idx, skinned_normal);
}

void main()
{
	vec3 p_skin;
	vec3 n_skin;
	do_skinning(position, normal, p_skin, n_skin);
	
	n = normal_to_camera * n_skin;
	l = mat3(world_to_camera) * light_direction;
	gl_Position = local_to_screen * vec4(p_skin, 1);


//	n = normal_to_camera * normal;
//	l = mat3(world_to_camera) * light_direction;  // tato verzia ignoruje posunutie
//	gl_Position = local_to_screen * vec4(position, 1);

}

#endif  //_VERTEX_

#ifdef _FRAGMENT_
precision mediump float;

varying vec3 n;  // normal in camera-space
varying vec3 l;  // light direction in camera-space

void main()
{
	vec3 n_norm = normalize(n);
	vec3 l_norm = normalize(l);
	float light = max(dot(n_norm, l_norm), 0.0) + 0.25;
	gl_FragColor = vec4(light * vec3(.7), 1);
}
#endif
