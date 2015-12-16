// zem s texturou
#include <string>
#include <cmath>
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/gles2/mesh_gles2.hpp"
#include "gl/gles2/program_gles2.hpp"
#include "gl/gles2/texture_loader_gles2.hpp"
#include "gl/gles2/touch_joystick_gles2.hpp"
#include "gl/gles2/default_shader_gles2.hpp"
#include "gl/window.hpp"
#include "gl/shapes.hpp"
#include "gl/colors.hpp"
#include "gl/controllers.hpp"

using std::string;
using glm::radians;
using glm::mat4;
using glm::vec4;
using glm::vec3;
using glm::ivec2;
using glm::translate;
using glm::scale;
using glm::angleAxis;
using glm::normalize;
using gl::camera;
using gl::free_camera;
using gl::make_sphere;
using gles2::shader::program;
using gles2::mesh;
using gles2::texture;
using gles2::texture2d;
using gles2::texture_filter;
using gles2::texture_from_file;
using gles2::ui::touch::joystick;
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


class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;
	scene_window();
	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	mesh _sphere;
	texture2d _earth_tex, _moon_tex;
	program _phong, _textured, _flat;
	float _earth_w, _moon_w, _sun_w;
	float _earth_ang, _moon_ang, _sun_ang;  // angles in radians
	bool _paused = false;
	gl::camera _cam;
	joystick _move_jstick, _look_jstick;
	float const _2pi;
};

scene_window::scene_window() 
	: _earth_w{radians(5.0f)}
	, _moon_w{radians(12.5f)}
	, _sun_w{radians(20.0f)}
	, _cam{radians(70.0f), aspect_ratio(), 0.01, 1000}
	, _move_jstick{ivec2{100, 500}, 50, width(), height()}
	, _look_jstick{ivec2{700, 500}, 50, width(), height()}
	, _2pi{2.0f * M_PI}
{
	_cam.position = vec3{0,0,95};
	_sphere = make_sphere<mesh>(1.0f, 120, 90);
	auto default_tex_params = texture::parameters{}.min(texture_filter::linear);
	_earth_tex = texture_from_file(earth_texture_path, default_tex_params);
	_moon_tex = texture_from_file(moon_texture_path, default_tex_params);
	_phong.from_memory(gles2::textured_phong_shader_source);
	_textured.from_memory(gles2::textured_shader_source);
	_flat.from_memory(gles2::flat_shader_source);
}

void scene_window::display()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_BACK, GL_LINE);

	vec4 const white{1};
	vec4 const black{0};
	vec4 const ambient{.05f, .05f, .05f, 1.0f};

	mat4 world_to_screen = _cam.world_to_screen();

	// sun
	auto & sun_prog = _flat;
	sun_prog.use();
	mat4 local_to_world = rotate(_sun_ang, vec3{0,-1,0}) * translate(vec3{90, 0, 0});
	mat4 local_to_screen = world_to_screen * local_to_world;
	sun_prog.uniform_variable("local_to_screen", local_to_screen);
	sun_prog.uniform_variable("color", rgb::yellow);
	_sphere.render();
	vec4 sun_pos = local_to_world[3];  // <-- toto je dobra finta local_to_world[3] vrati posunutie transformacie

	// earth
	auto & earth_prog = _phong;
//	auto & earth_prog = _textured;
//	auto & earth_prog = _solid;
	earth_prog.use();
	local_to_world = rotate(_earth_ang, vec3{0,1,0}) * scale(vec3{12.756});
	local_to_screen = world_to_screen * local_to_world;
	earth_prog.uniform_variable("local_to_screen", local_to_screen);
	earth_prog.uniform_variable("local_to_world", local_to_world);
	earth_prog.uniform_variable("world_eye_pos", vec4{_cam.position, 1});
	earth_prog.uniform_variable("world_light_pos", sun_pos);
	earth_prog.uniform_variable("light_color", white);
	earth_prog.uniform_variable("material_emissive", black);
	earth_prog.uniform_variable("material_diffuse", white);
	earth_prog.uniform_variable("material_specular", white);
	earth_prog.uniform_variable("material_shininess", 50.0f);
	earth_prog.uniform_variable("ambient", ambient);
	_earth_tex.bind(0);
	earth_prog.uniform_variable("s", 0);
//	earth_prog.uniform_variable("color", rgb::blue);
	_sphere.render();

//	// moon
	auto & moon_prog = _phong;
//	auto & moon_prog = _textured;
//	auto & moon_prog = _solid;
	moon_prog.use();
	local_to_world = rotate(_moon_ang, vec3{0,1,0}) * translate(vec3{60, 0, 0}) * scale(vec3{3.476});
	local_to_screen = world_to_screen * local_to_world;
	moon_prog.uniform_variable("local_to_screen", local_to_screen);
	moon_prog.uniform_variable("local_to_world", local_to_world);
	_moon_tex.bind(0);
	moon_prog.uniform_variable("s", 0);
//	moon_prog.uniform_variable("color", rgb::gray);
	_sphere.render();

	_move_jstick.render();
	_look_jstick.render();

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

	// update move-joystick
	if (in.mouse(button::left))
	{
		_move_jstick.touch(in.mouse_position(), joystick::touch_event::down);
		_look_jstick.touch(in.mouse_position(), joystick::touch_event::down);
	}
	else
	{
		_move_jstick.touch(in.mouse_position(), joystick::touch_event::up);
		_look_jstick.touch(in.mouse_position(), joystick::touch_event::up);
	}

	// move-joystick use
	float linear_velocity = 10.0;
	if (_move_jstick.up())
		_cam.position -= linear_velocity*dt * _cam.forward();
	if (_move_jstick.down())
		_cam.position += linear_velocity*dt * _cam.forward();
	if (_move_jstick.left())
		_cam.position -= linear_velocity*dt * _cam.right();
	if (_move_jstick.right())
		_cam.position += linear_velocity*dt * _cam.right();

	// look-joystick use
	float angular_velocity = .5;
	if (_look_jstick.up())
		_cam.rotation = normalize(angleAxis(angular_velocity*dt, _cam.right()) * _cam.rotation);
	if (_look_jstick.down())
		_cam.rotation = normalize(angleAxis(-angular_velocity*dt, _cam.right()) * _cam.rotation);
	if (_look_jstick.left())
		_cam.rotation = normalize(angleAxis(angular_velocity*dt, _cam.up()) * _cam.rotation);
	if (_look_jstick.right())
		_cam.rotation = normalize(angleAxis(-angular_velocity*dt, _cam.up()) * _cam.rotation);

	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
