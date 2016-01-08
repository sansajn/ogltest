// citanie md5 modelu
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <cassert>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/glew.h>
#include "gl/program.hpp"
#include "gl/controllers.hpp"
#include "gl/animation.hpp"
#include "gl/glut_window.hpp"
#include "gl/shapes.hpp"

using std::swap;
using std::string;
using std::vector;
using std::make_pair;
using std::move;
using std::shared_ptr;
using std::unique_ptr;
using std::max;
using std::min;
using std::cout;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec4;
using glm::mat3;
using glm::mat4;
using glm::cross;
using glm::normalize;
using glm::radians;
using glm::inverseTranspose;
using glm::inverse;
using glm::rotate;
using glm::translate;
using glm::scale;
using glm::mat4_cast;
using gl::skeletal_animation;
using gl::mesh;
using gl::free_camera;
using gl::animated_model;
using gl::animated_model_from_file;
using gl::camera;

// quake 4 blaster
string model_path = "assets/blaster/view.md5mesh";

string anim_paths[] = {
	"assets/blaster/big_recoil.md5anim",
	"assets/blaster/charge_up.md5anim",
	"assets/blaster/fire.md5anim",
	"assets/blaster/fire2.md5anim",
	"assets/blaster/flashlight.md5anim",
	"assets/blaster/idle.md5anim",
	"assets/blaster/lower.md5anim",
	"assets/blaster/raise.md5anim"
};

string skinned_shader_program = "shaders/bump_skinned.glsl";
string axis_shader_path = "shaders/colored.glsl";
string light_shader_path = "shaders/solid.glsl";


class player    // player_object
{
public:
	enum {  // animationas
		big_recoil_animation,
		charge_up_animation,
		fire_animation,
		fire2_animation,
		flashlight_animation,
		idle_animation,
		lower_animation,
		raise_animation
	};

	enum class state {
		idle,
		fire,
	};

	void init();
	void update(float dt);
	void render(shader::program & prog);

	// states
	void fire();
	void idle();

	// docastne
	std::vector<glm::mat4> const & skeleton() const {return _mdl.skeleton();}

private:
	animated_model _mdl;
	state _state;
};

void player::init()
{
	auto model_params = gl::model_loader_parameters{};
	model_params.file_format = ".tga";
	model_params.diffuse_texture_postfix = "_d";
	_mdl = animated_model_from_file(model_path, model_params);

	for (string const & anim_path : anim_paths)
		_mdl.append_animation(skeletal_animation{anim_path});

	_mdl.animation_sequence(vector<unsigned>{idle_animation});

	_state = state::idle;
}

void player::update(float dt)
{
	// state update
	if (_state == state::fire)
	{
		if (_mdl.animation_state() == animated_model::state::done)
			idle();
	}

	_mdl.update(dt);
}

void player::render(shader::program & prog)
{
	_mdl.render(prog);
}

void player::fire()
{
	_state = state::fire;
	_mdl.animation_sequence(vector<unsigned>{fire_animation}, animated_model::repeat_mode::once);  // TODO: specializovana funkcia pre sekvencie dlzky 1
}

void player::idle()
{
	_state = state::idle;
	_mdl.animation_sequence(vector<unsigned>{idle_animation}, animated_model::repeat_mode::loop);
}


class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void update(float dt) override;
	void display() override;
	void input(float dt) override;

private:
	shader::program _prog;
	player _player;
	free_camera<scene_window> _view;

	// debug
	mesh _axis;
	shader::program _axis_prog;
	mesh _light;
	shader::program _light_prog;
};

scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_player.init();

	_view.get_camera().look_at(vec3{1,0,0});

	_prog.from_file(skinned_shader_program);

	_axis = gl::make_axis<mesh>();
	_axis_prog.from_file(axis_shader_path);
	_light = gl::make_sphere<mesh>();
	_light_prog.from_file(light_shader_path);

	glClearColor(0,0,0,1);
}

void scene_window::update(float dt)
{
	base::update(dt);
	_player.update(dt);
}

void scene_window::display()
{
	camera & cam = _view.get_camera();
	mat4 M = mat4{1};
	M = rotate(M, radians(-90.0f), vec3{1, 0, 0});
	mat4 world_to_camera = cam.view();
	mat4 local_to_screen = cam.projection() * world_to_camera * M;
	mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera * M)};

	vec3 light_pos = vec3{5,5,-5};

	_prog.use();
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("world_to_camera", world_to_camera);
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_camera", normal_to_camera);
	_prog.uniform_variable("light.direction", normalize(light_pos));
	_prog.uniform_variable("skeleton", _player.skeleton());

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	_player.render(_prog);

	// light
	mat4 M_light = translate(mat4{1}, light_pos);
	M_light = scale(M_light, vec3{0.1, 0.1, 0.1});
	_light_prog.use();
	_light_prog.uniform_variable("color", vec3{1,1,0});  // yellow
	_light_prog.uniform_variable("local_to_screen", cam.view_projection() * M_light);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_light.render();

	// axis
	_axis_prog.use();
	_axis_prog.uniform_variable("local_to_screen", cam.view_projection());
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_axis.render();

	base::display();
}

void scene_window::input(float dt)
{
	_view.input(dt);

	if (in.key_up(' '))
		_player.fire();

	base::input(dt);
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
