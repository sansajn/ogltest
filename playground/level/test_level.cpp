// renderuje level
#include <vector>
#include <string>
#include <glm/gtc/matrix_inverse.hpp>
#include "window.hpp"
#include "program.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "scene_object.hpp"
#include "level.hpp"

using std::vector;
using std::string;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using gl::mesh;
using gl::camera;
using gl::free_look;
using gl::free_move;
using ui::glut_pool_window;


class fps_move  //!< wsad move in xz plane
{
public:
	fps_move(camera & c, glut_pool_window & w, float speed = 0.1);
	void input(float dt);
	void speed(float v) {_speed = v;}
	void controls(char forward, char backward, char left, char right);

private:
	enum class key {forward, backward, left, right, key_count};
	camera & _cam;
	glut_pool_window & _wnd;
	float _speed;
	char _controls[(int)key::key_count];
};

fps_move::fps_move(camera & c, glut_pool_window & w, float speed)
	: _cam{c}, _wnd{w}, _speed{speed}
{
	controls('w', 's', 'a', 'd');
}

void fps_move::input(float dt)
{
	vec3 fwd_dir = _cam.forward();
	fwd_dir.y = 0;

	if (_wnd.in.key(_controls[int(key::forward)]))
		_cam.position -= fwd_dir * _speed;

	if (_wnd.in.key(_controls[int(key::backward)]))
		_cam.position += fwd_dir * _speed;

	if (_wnd.in.key(_controls[int(key::left)]))
		_cam.position -= _cam.right() * _speed;

	if (_wnd.in.key(_controls[int(key::right)]))
		_cam.position += _cam.right() * _speed;
}

void fps_move::controls(char forward, char backward, char left, char right)
{
	_controls[(int)key::forward] = forward;
	_controls[(int)key::backward] = backward;
	_controls[(int)key::left] = left;
	_controls[(int)key::right] = right;
}


struct view  // view properties
{
	float fovy;
	float aspect_ratio;
	float near;
	float far;
};

class player
{
public:
	player(glut_pool_window & w);
	void position(vec3 const & p);
	void view_parameters(view const & v);  // TODO: view properties
	camera & get_camera() {return _cam;}
	camera const & get_camera() const {return _cam;}
	void input(float dt);

private:
	camera _cam;
	glut_pool_window * _wnd;
	free_look<glut_pool_window> _look;
	fps_move _move;
};

player::player(glut_pool_window & w)
	: _wnd{&w}, _look{_cam, w}, _move{_cam, w}
{}

void player::position(vec3 const & p)
{
	_cam.position = p;
}

void player::view_parameters(view const & v)
{
	_cam = camera{v.fovy, v.aspect_ratio, v.near, v.far};
}

void player::input(float dt)
{
	_look.input(dt);
	_move.input(dt);
}


class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	level _lvl;
	player _player;
	axis_object _axis;
	light_object _light;
};

scene_window::scene_window()
	: _player{*this}
{
	// TODO: treba volat v spravnom poradi
	view v;
	v.fovy = radians(70.0);
	v.aspect_ratio = aspect_ratio();
	v.near = 0.01;
	v.far = 1000.0;
	_player.view_parameters(v);
	_player.position(_lvl.player_position());

	glClearColor(0, 0, 0, 1);
}

void scene_window::display()
{
	vec3 const light_pos{3,5,3};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_lvl.render(_player.get_camera());
	_axis.render(_player.get_camera());
	_light.render(_player.get_camera(), light_pos);

	base::display();
}

void scene_window::input(float dt)
{
	_player.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
