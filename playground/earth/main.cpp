// zem s texturou
#include <string>
#include <cmath>
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/window.hpp"
#include "gl/shapes.hpp"
#include "gl/texture_loader.hpp"
#include "gl/controllers.hpp"
#include "gl/colors.hpp"
#include "gl/program.hpp"

using std::string;
using glm::radians;
using glm::mat4;
using glm::vec4;
using glm::vec3;
using glm::translate;
using glm::scale;
using gl::free_camera;
using gl::mesh;
using gl::make_sphere;
using gl::texture_from_file;
using ui::glut_pool_window;

char const * solid_shader_path = "assets/shaders/solid.glsl";
char const * textured_shader_path = "assets/shaders/textured.glsl";
char const * earth_texture_path = "assets/textures/1_earth_1k.jpg";
//char const * earth_texture_path = "assets/textures/1_earth_8k.jpg";
//char const * earth_texture_path = "assets/textures/1_earth_16k.jpg";
//char const * earth_texture_path = "assets/textures/2_no_clouds_8k.jpg";
//char const * earth_texture_path = "assets/textures/2_no_clouds_16k.jpg";
char const * moon_texture_path = "assets/textures/moonmap1k.jpg";
//char const * moon_texture_path = "assets/textures/moonmap4k.jpg";

char const * phong_shader_source = R"(
	// phong implementacia (pocitana vo world priestore)
	//#version 330
	uniform mat4 local_to_screen;
	uniform mat4 local_to_world;
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texcoord;
	layout(location = 2) in vec3 normal;
	out VS_OUT {
		vec4 world_position;  // world space position
		vec4 world_normal;  // world space normal
		vec2 texcoord;
	} vs_out;
	void main() {
		vs_out.world_position = local_to_world * vec4(position, 1);
		vs_out.world_normal = local_to_world * vec4(normal, 0);
		vs_out.texcoord = texcoord;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	uniform vec4 world_eye_pos;  // eye position in world space
	uniform vec4 world_light_pos;
	uniform vec4 light_color;  // light's diffuse and specular contribution
	uniform vec4 material_emissive;
	uniform vec4 material_diffuse;
	uniform vec4 material_specular;
	uniform float material_shininess;
	uniform vec4 ambient;  // global ambient
	uniform sampler2D s;

	in VS_OUT {
		vec4 world_position;  // world space position
		vec4 world_normal;  // world space normal
		vec2 texcoord;
	} fs_in;

	out vec4 fcolor;

	void main() {
		// emissive term
		vec4 emissive = material_emissive;

		// diffuse term
		vec4 N = normalize(fs_in.world_normal);
		vec4 L = normalize(world_light_pos - fs_in.world_position);
		float NdotL = max(dot(N,L), 0);
		vec4 diffuse = NdotL * light_color * material_diffuse;

		// specular
		vec4 V = normalize(world_eye_pos - fs_in.world_position);
		vec4 H = normalize(L+V);
		vec4 R = reflect(-L,N);
		float RdotV = max(dot(R,V), 0);
		float NdotH = max(dot(N,H), 0);
		vec4 specular = pow(RdotV, material_shininess) * light_color * material_specular;

		fcolor = (emissive + ambient + diffuse + specular) * texture(s, fs_in.texcoord);
	}
	#endif
)";

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;
	scene_window();
	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	free_camera<scene_window> _cam;
	mesh _sphere;
	texture2d _earth_tex, _moon_tex;
	shader::program _phong;
	shader::program _textured;
	shader::program _solid;
	float _earth_w, _moon_w, _sun_w;
	float _earth_ang, _moon_ang, _sun_ang;  // angles in radians
	bool _paused = false;
	float const _2pi;
};

scene_window::scene_window() 
	: _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
	, _earth_w{radians(5.0f)}
	, _moon_w{radians(12.5f)}
	, _sun_w{radians(20.0f)}
	, _2pi{2.0f * M_PI}
{
	_cam.get_camera().position = vec3{0,0,95};
	_sphere = make_sphere(1.0f, 120, 90);
	auto default_tex_params = texture::parameters{}.min(texture_filter::linear_mipmap_linear);
	_earth_tex = texture_from_file(earth_texture_path, default_tex_params);
	_moon_tex = texture_from_file(moon_texture_path, default_tex_params);
	_phong.from_memory(phong_shader_source);
	_textured.from_file(textured_shader_path, 420);
	_solid.from_file(solid_shader_path, 420);
}

void scene_window::display()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_BACK, GL_LINE);

	vec4 const white{1};
	vec4 const black{0};
	vec4 const ambient{.1f, .1f, .1f, 1.0f};

	mat4 world_to_screen = _cam.get_camera().world_to_screen();

	// sun
	auto & sun_prog = _solid;
	sun_prog.use();
	mat4 local_to_world = rotate(_sun_ang, vec3{0,-1,0}) * translate(vec3{90, 0, 0});
	mat4 local_to_screen = world_to_screen * local_to_world;
	sun_prog.uniform_variable("local_to_screen", local_to_screen);
	sun_prog.uniform_variable("color", rgb::yellow);
	_sphere.render();
	vec4 sun_pos = local_to_world[3];  // <-- toto je dobra finta local_to_world[3] vrati posunutie transformacie

	// earth
	auto & earth_prog = _phong;
	earth_prog.use();
	local_to_world = rotate(_earth_ang, vec3{0,1,0}) * scale(vec3{12.756});
	local_to_screen = world_to_screen * local_to_world;
	earth_prog.uniform_variable("local_to_screen", local_to_screen);
	earth_prog.uniform_variable("local_to_world", local_to_world);
	earth_prog.uniform_variable("world_eye_pos", vec4{_cam.get_camera().position, 1});
	earth_prog.uniform_variable("world_light_pos", sun_pos);
	earth_prog.uniform_variable("light_color", white);
	earth_prog.uniform_variable("material_emissive", black);
	earth_prog.uniform_variable("material_diffuse", white);
	earth_prog.uniform_variable("material_specular", white);
	earth_prog.uniform_variable("material_shininess", 50.0f);
	earth_prog.uniform_variable("ambient", ambient);
	_earth_tex.bind(0);
	earth_prog.uniform_variable("s", 0);
	_sphere.render();

	// moon
	auto & moon_prog = _phong;
	moon_prog.use();
	local_to_world = rotate(_moon_ang, vec3{0,1,0}) * translate(vec3{60, 0, 0}) * scale(vec3{3.476});
	local_to_screen = world_to_screen * local_to_world;
	moon_prog.uniform_variable("local_to_screen", local_to_screen);
	moon_prog.uniform_variable("local_to_world", local_to_world);
	_moon_tex.bind(0);
	moon_prog.uniform_variable("s", 0);
	_sphere.render();

	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);
	if (_paused)
		return;
	_earth_ang = fmod(_earth_ang + _earth_w * dt, _2pi);
	_moon_ang = fmod(_moon_ang + _moon_w * dt, _2pi);
	_sun_ang = fmod(_sun_ang + _sun_w * dt, _2pi);
}

void scene_window::input(float dt)
{
	if (in.key_up(' '))
		_paused = _paused ? false : true;
	_cam.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
