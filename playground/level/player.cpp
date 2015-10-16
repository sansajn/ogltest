#include "player.hpp"

using glm::vec3;
using gl::camera;
using ui::glut_pool_window;

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

void player::update(float dt)
{
	_prev_pos = _cam.position;
}
