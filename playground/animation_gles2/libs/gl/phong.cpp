#include "phong.hpp"
#include <glm/gtc/matrix_inverse.hpp>
#include "gl/colors.hpp"

namespace shader {

using glm::vec3;
using glm::mat4;
using glm::mat3;
using glm::inverseTranspose;
using gl::mesh;
using gl::camera;

void phong_light::init(shader::program * p, camera const * cam)
{
	assert(!_prog && !_cam && "already initialized");
	_prog = p;
	_cam = cam;
	light(normalize(vec3{10,20,30}), rgb::white, 1.0f);
	material(vec3{.1}, 16.0f, .4f);
}

void phong_light::render(mesh const & m, mat4 const & local_to_world)
{
	_prog->use();
	mat4 V = _cam->world_to_camera();
	_prog->uniform_variable("camera_to_screen", _cam->camera_to_screen());
	_prog->uniform_variable("world_to_camera", V);
	_prog->uniform_variable("local_to_world", local_to_world);
	_prog->uniform_variable("normal_to_camera", mat3{inverseTranspose(V*local_to_world)});
	// ostatne sa nastavi volanim light() a material()
	m.render();
}

void phong_light::light(vec3 const & direction, vec3 const & color, float intensity)
{
	_prog->use();
	_prog->uniform_variable("light.direction", direction);
	_prog->uniform_variable("light.color", color);
	_prog->uniform_variable("light.intensity", intensity);
}

void phong_light::material(vec3 const & ambient, float shininess, float intensity)
{
	_prog->use();
	_prog->uniform_variable("material.ambient", ambient);
	_prog->uniform_variable("material.shininess", shininess);
	_prog->uniform_variable("material.intensity", intensity);
}

void phong_light::light_direction(vec3 const & dir)
{
	_prog->use();
	_prog->uniform_variable("light.direction", dir);
}

char const * phong_shader_source = R"(
	// phongov osvetlovaci model (pocitane s priestoru kamery)
	struct directional_light
	{
		vec3 direction;
		vec3 color;
		float intensity;
	};

	struct material_prop
	{
		vec3 ambient;
		float shininess;
		float intensity;  // shininess intensity
	};

	uniform mat4 camera_to_screen;
	uniform mat4 world_to_camera;
	uniform mat4 local_to_world;
	uniform mat3 normal_to_camera;
	uniform directional_light light;
	uniform material_prop material;
	uniform vec3 color = vec3(.7, .7, .7);

	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=2) in vec3 normal;

	out VS_OUT {
		vec3 v;  // view-direction in camera space (not normalized)
		vec3 l;  // light-direction in camera space (not normalized)
		vec3 n;  // normal-direction in camera space (not normalized)
	} vs_out;

	void main()
	{
		mat4 local_to_camera = world_to_camera * local_to_world;
		mat4 local_to_screen = camera_to_screen * local_to_camera;

		vs_out.n = normalize(normal_to_camera * normal);

		vec4 p = local_to_camera * vec4(position,1);
		vs_out.v = -p.xyz;  // view in camera space

		vec3 l = vec3(world_to_camera * vec4(light.direction,0));
		vs_out.l = l;  // light in camera space

		gl_Position = local_to_screen * vec4(position,1);
	}

	#endif
	#ifdef _FRAGMENT_
	in VS_OUT {
		vec3 v;  // view-direction in camera space (not normalized)
		vec3 l;  // light-direction in camera space (not normalized)
		vec3 n;  // normal-direction in camera space (not normalized)
	} fs_in;

	out vec4 fcolor;

	void main()
	{
		vec3 v = normalize(fs_in.v);
		vec3 l = normalize(fs_in.l);
		vec3 n = normalize(fs_in.n);
		vec3 r = normalize(-reflect(l,n));

		vec3 amb = material.ambient;
		float diff = max(dot(n,l), 0.0) * light.intensity;
		float spec = pow(max(dot(r,v), 0.0), material.shininess) * material.intensity * light.intensity;

		fcolor = vec4(amb*light.color + (diff+spec) * light.color, 1) * vec4(color, 1);
	}
	#endif
)";

char const * phong_textured_shader_source = R"(
	// phongov osvetlovaci model (pocitane s priestoru kamery) s texturou
	struct directional_light
	{
		vec3 direction;
		vec3 color;
		float intensity;
	};

	struct material_prop
	{
		vec3 ambient;
		float shininess;
		float intensity;  // shininess intensity
	};

	uniform mat4 camera_to_screen;
	uniform mat4 world_to_camera;
	uniform mat4 local_to_world;
	uniform mat3 normal_to_camera;
	uniform directional_light light;
	uniform material_prop material;
	uniform sampler2D s;

	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=1) in vec2 uv;
	layout(location=2) in vec3 normal;

	out VS_OUT {
		vec3 v;  // view-direction in camera space (not normalized)
		vec3 l;  // light-direction in camera space (not normalized)
		vec3 n;  // normal-direction in camera space (not normalized)
		vec2 uv;
	} vs_out;

	void main()
	{
		mat4 local_to_camera = world_to_camera * local_to_world;
		mat4 local_to_screen = camera_to_screen * local_to_camera;

		vs_out.n = normalize(normal_to_camera * normal);

		vec4 p = local_to_camera * vec4(position,1);
		vs_out.v = -p.xyz;  // view in camera space

		vec3 l = vec3(world_to_camera * vec4(light.direction,0));
		vs_out.l = l;  // light in camera space

		vs_out.uv = uv;
		gl_Position = local_to_screen * vec4(position,1);
	}

	#endif
	#ifdef _FRAGMENT_
	in VS_OUT {
		vec3 v;  // view-direction in camera space (not normalized)
		vec3 l;  // light-direction in camera space (not normalized)
		vec3 n;  // normal-direction in camera space (not normalized)
		vec2 uv;
	} fs_in;

	out vec4 fcolor;

	void main()
	{
		vec3 v = normalize(fs_in.v);
		vec3 l = normalize(fs_in.l);
		vec3 n = normalize(fs_in.n);
		vec3 r = normalize(-reflect(l,n));

		vec3 amb = material.ambient;
		float diff = max(dot(n,l), 0.0) * light.intensity;
		float spec = pow(max(dot(r,v), 0.0), material.shininess) * material.intensity * light.intensity;

		fcolor = vec4(amb*light.color + (diff+spec) * light.color, 1) * texture(s, fs_in.uv);
	}
	#endif
)";

}  // shader
