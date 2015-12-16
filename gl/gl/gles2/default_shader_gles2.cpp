#include "default_shader_gles2.hpp"

namespace gles2 {

char const * flat_shader_source = R"(
	// zobrazi model v zakladnej farbe bez osvetlenia
	uniform mat4 local_to_screen;
	uniform vec3 color = vec3(.7);
	#ifdef _VERTEX_
	attribute vec3 position;
	void main()	{
		gl_Position = local_to_screen * vec4(position,1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	void main() {
		gl_FragColor = vec4(color, 1);
	}
	#endif
)";

char const * textured_shader_source = R"(
	// texturovany model
	uniform mat4 local_to_screen;
	uniform sampler2D s;
	#ifdef _VERTEX_
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
	varying vec2 uv;
	void main() {
		gl_FragColor = texture2D(s, uv);
	}
	#endif
)";

char const * textured_phong_shader_source = R"(
	// phong implementacia (pocitana vo world priestore)
	uniform mat4 local_to_screen;
	uniform mat4 local_to_world;
	uniform vec4 world_eye_pos;  // eye position in world space
	uniform vec4 world_light_pos;
	uniform vec4 light_color;  // light's diffuse and specular contribution
	uniform vec4 material_emissive;
	uniform vec4 material_diffuse;
	uniform vec4 material_specular;
	uniform float material_shininess;
	uniform vec4 ambient;  // global ambient
	uniform sampler2D s;
	#ifdef _VERTEX_
	attribute vec3 position;
	attribute vec2 texcoord;
	attribute in vec3 normal;
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
	varying vec4 vs_world_position;  // world space position
	varying vec4 vs_world_normal;  // world space normal
	varying vec2 vs_texcoord;

	void main() {
		// emissive term
		vec4 emissive = material_emissive;

		// diffuse term
		vec4 N = normalize(vs_world_normal);
		vec4 L = normalize(world_light_pos - vs_world_position);
		float NdotL = max(dot(N,L), 0);
		vec4 diffuse = NdotL * light_color * material_diffuse;

		// specular
		vec4 V = normalize(world_eye_pos - vs_world_position);
		vec4 H = normalize(L+V);
		vec4 R = reflect(-L,N);
		float RdotV = max(dot(R,V), 0);
		float NdotH = max(dot(N,H), 0);
		vec4 specular = pow(RdotV, material_shininess) * light_color * material_specular;

		gl_FragColor = (emissive + ambient + diffuse + specular) * texture2D(s, vs_texcoord);
	}
	#endif
)";

}  // gles2
