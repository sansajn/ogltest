#include "default_shader_gles2.hpp"

namespace gles2 {

/* Poznamky k implementacii gles2 shaderov.
note: ak su uniformi definovane pred #ifdef, potom na nexuse7 nejde fragment shader skompilovat (error C7573: OpenGL/ES requires precision specifier on float types)
note: opengl es2 nepodporuje defaultne hodnoty uniformou */

//! zobrazi model v zakladnej farbe bez osvetlenia
char const * flat_shader_source = R"(
	#ifdef _VERTEX_
	uniform mat4 local_to_screen;
	attribute vec3 position;
	void main()	{
		gl_Position = local_to_screen * vec4(position,1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	uniform vec3 color;  // vec3(.7);
	void main() {
		gl_FragColor = vec4(color, 1);
	}
	#endif
)";

//! zozbrazi model s tienovanim zalozenom na normale vrchola
char const * flat_shaded_shader_source = R"(
	#ifdef _VERTEX_
	attribute vec3 position;
	attribute vec3 normal;
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	varying vec3 n;
	void main() {
		n = normal_to_world * normal;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	uniform vec3 color;  // vec3(0.7, 0.7, 0.7)
	uniform vec3 light_dir;  // normalize(vec3(1,2,3))
	varying vec3 n;
	void main() {
		gl_FragColor = vec4(max(dot(n, light_dir), 0.2) * color, 1);
	}
	#endif
)";

//! texturovany model
char const * textured_shader_source = R"(
	#ifdef _VERTEX_
	uniform mat4 local_to_screen;
	attribute vec3 position;
	attribute vec2 texcoord;
	varying vec2 uv;
	void main() {
		uv = texcoord;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	uniform sampler2D s;
	varying vec2 uv;
	void main() {
		gl_FragColor = texture2D(s, uv);
	}
	#endif
)";

//! phong implementacia (pocitana vo world priestore)
char const * textured_phong_shader_source = R"(
	#ifdef _VERTEX_
	attribute vec3 position;
	attribute vec2 texcoord;
	attribute in vec3 normal;
	uniform mat4 local_to_screen;
	uniform mat4 local_to_world;
	varying vec4 vs_world_position;  // world space position
	varying vec4 vs_world_normal;  // world space normal
	varying vec2 vs_texcoord;

	void main() {
		vs_world_position = local_to_world * vec4(position, 1);
		vs_world_normal = local_to_world * vec4(normal, 0);
		vs_texcoord = texcoord;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	uniform vec4 world_eye_pos;  // eye position in world space
	uniform vec4 world_light_pos;
	uniform vec4 light_color;  // light's diffuse and specular contribution
	uniform vec4 material_emissive;
	uniform vec4 material_diffuse;
	uniform vec4 material_specular;
	uniform float material_shininess;
	uniform vec4 ambient;  // global ambient
	uniform sampler2D s;
	varying vec4 vs_world_position;  // world space position
	varying vec4 vs_world_normal;  // world space normal
	varying vec2 vs_texcoord;

	void main() {
		// emissive term
		vec4 emissive = material_emissive;

		// diffuse term
		vec4 N = normalize(vs_world_normal);
		vec4 L = normalize(world_light_pos - vs_world_position);
		float NdotL = max(dot(N,L), 0.0);
		vec4 diffuse = NdotL * light_color * material_diffuse;

		// specular
		vec4 V = normalize(world_eye_pos - vs_world_position);
		vec4 H = normalize(L+V);
		vec4 R = reflect(-L,N);
		float RdotV = max(dot(R,V), 0.0);
		float NdotH = max(dot(N,H), 0.0);
		vec4 specular = pow(RdotV, material_shininess) * light_color * material_specular;

		gl_FragColor = (emissive + ambient + diffuse + specular) * texture2D(s, vs_texcoord);
	}
	#endif
)";


/*! GPU kosterna animacia s tienovanim (diffuse light).
\note gles2 zarucuje minimalne 128 vec4 uniformou pre vertex shader (tegra3 ich podporuje 256) */
char const * skinned_flat_shaded_shader_source = R"(
#ifdef _VERTEX_
	const int MAX_JOINTS = 64;

	attribute vec3 position;  // bind pose positions
	attribute vec3 normal;
	attribute vec4 joints;  // skeleton indices
	attribute vec4 weights;

	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform mat4 skeleton[MAX_JOINTS];  // kostra, ako zoznam transformacii

	varying vec3 n;

	void main()
	{
		mat4 T_skin =
			skeleton[int(joints.x)] * weights.x +
			skeleton[int(joints.y)] * weights.y +
			skeleton[int(joints.z)] * weights.z +
			skeleton[int(joints.w)] * weights.w;

		vec4 n_skin = T_skin * vec4(normal, 0);
		n = normal_to_world * n_skin.xyz;

		gl_Position = local_to_screen * T_skin * vec4(position, 1);
	}
#endif  // _VERTEX_

#ifdef _FRAGMENT_
	precision mediump float;
	uniform vec3 color;
	varying vec3 n;
	vec3 light_direction = normalize(vec3(1,2,3));

	void main()
	{
		float light = clamp(dot(normalize(n), light_direction), 0.2, 1);
		gl_FragColor = vec4(light * color, 1);
	}
#endif  // _FRAGMENT_
)";

}  // gles2
