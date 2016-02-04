#pragma once
#include <functional>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "gl/camera.hpp"
#include "gl/window.hpp"

namespace gl {

class camera_controller
{
public:
	virtual void input(float dt) = 0;
	virtual ~camera_controller() {}
};

template <typename PoolWindow>  //!< \sa ui::window<ui::glut_pool_impl>
class free_move : public camera_controller
{
public:
	free_move(camera & c, PoolWindow & w, float movement = 0.1f);
	void input(float dt) override;
	void movement(float v) {_movement = v;}
	void controls(char forward, char backward, char left, char right, char up = '\0', char down = '\0');
	void assoc_camera(camera & cam) {_cam = &cam;}
	void set_move_callback(std::function<void ()> callback);

protected:
	camera * _cam;
	PoolWindow & _wnd;

private:
	enum class key {up, down, left, right, forward, backward};

	float _movement;
	char _keys[6];
	std::function<void ()> _move_callback;
};

template <typename PoolWindow>
class map_move : public free_move<PoolWindow>
{
public:
	using base = free_move<PoolWindow>;

	// TODO: nastavuj movement podla vzdialenosti (ak som dalej tak viac, inak menej)
	map_move(camera & c, PoolWindow & w, float movement = 0.1f);
	void input(float dt) override;
};

template <typename PoolWindow>  //!< \sa ui::window<ui::glut_pool_impl>
class free_look : public camera_controller
{
public:
	free_look(camera & c, PoolWindow & w, glm::vec3 const & up = glm::vec3(0,1,0)) : _cam(&c), _wnd(w), _up(up) {}
	void input(float dt) override;
	void assoc_camera(camera & cam) {_cam = &cam;}  // TODO: naco je assoc_cam ak free_look nemozem vytvorit bez kamery ?
	bool enabled() const {return _enabled;}

private:
	camera * _cam;
	PoolWindow & _wnd;
	glm::vec3 _up;
	bool _enabled = false;
};

template <typename Window>  //!< \sa ui::window<ui::glut_pool_impl>
class free_camera : public camera_controller
{
public:
	free_camera(float fovy, float aspect, float near, float far, Window & w);
	camera & get_camera() {return _cam;}  // TODO: premenuj na camera_ref ?
	void input(float dt) override;

private:
	camera _cam;
	free_move<Window> _move;
	free_look<Window> _look;
};

template <typename PoolWindow>
free_move<PoolWindow>::free_move(camera & c, PoolWindow & w, float movement)
	: _cam(&c), _wnd(w), _movement(movement)
{
	controls('w', 's', 'a', 'd');
}

template <typename PoolWindow>
void free_move<PoolWindow>::controls(char forward, char backward, char left, char right, char up, char down)
{
	_keys[int(key::forward)] = forward;
	_keys[int(key::backward)] = backward;
	_keys[int(key::left)] = left;
	_keys[int(key::right)] = right;
	_keys[int(key::up)] = up;
	_keys[int(key::down)] = down;
}

template <typename PoolWindow>
void free_move<PoolWindow>::set_move_callback(std::function<void ()> callback)
{
	_move_callback = callback;
}

template <typename PoolWindow>
void free_look<PoolWindow>::input(float dt)
{
	glm::ivec2 center = _wnd.center();

	if (_wnd.in.mouse(ui::event_handler::button::left) && !_enabled)
	{
		_enabled = true;
		glutSetCursor(GLUT_CURSOR_NONE);
		glutWarpPointer(center.x, center.y);
		return;
	}

	if (_wnd.in.key_up(27) && _enabled)  // esc
	{
		_enabled = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}

	if (!_enabled)
		return;

	float const angular_movement = 0.1f;

	glm::ivec2 delta = _wnd.in.mouse_position() - center;

	if (delta.x != 0)
	{
		float angle = glm::radians(angular_movement * delta.x);
		_cam->rotation = glm::normalize(glm::angleAxis(-angle, _up) * _cam->rotation);
	}

	if (delta.y != 0)
	{
		float angle = glm::radians(angular_movement * delta.y);
		_cam->rotation = glm::normalize(glm::angleAxis(-angle, _cam->right()) * _cam->rotation);
	}

	if (delta.x != 0 || delta.y != 0)
		glutWarpPointer(center.x, center.y);
}

template <typename PoolWindow>
map_move<PoolWindow>::map_move(camera & c, PoolWindow & w, float movement)
	: base(c, w, movement)
{
	base::controls('\0', '\0', 'a', 'd', 'w', 's');
}

template <typename PoolWindow>
void map_move<PoolWindow>::input(float dt)
{
	if (base::_wnd.in.wheel_up(ui::event_handler::wheel::up))
		base::_cam->position.z /= 1.1f;

	if (base::_wnd.in.wheel_up(ui::event_handler::wheel::down))
		base::_cam->position.z *= 1.1f;

	base::input(dt);
}

template <typename PoolWindow>
void free_move<PoolWindow>::input(float dt)
{
	bool moved = false;

	if (_wnd.in.key(_keys[int(key::up)]))
	{
		_cam->position += _cam->up() * _movement;
		moved = true;
	}

	if (_wnd.in.key(_keys[int(key::down)]))
	{
		_cam->position -= _cam->up() * _movement;
		moved = true;
	}

	if (_wnd.in.key(_keys[int(key::left)]))
	{
		_cam->position -= _cam->right() * _movement;
		moved = true;
	}

	if (_wnd.in.key(_keys[int(key::right)]))
	{
		_cam->position += _cam->right() * _movement;
		moved = true;
	}

	if (_wnd.in.key(_keys[int(key::forward)]))
	{
		_cam->position -= _cam->forward() * _movement;
		moved = true;
	}

	if (_wnd.in.key(_keys[int(key::backward)]))
	{
		_cam->position += _cam->forward() * _movement;
		moved = true;
	}

	if (moved && _move_callback)
		_move_callback();
}

template <typename Window>
free_camera<Window>::free_camera(float fovy, float aspect, float near, float far, Window & w)
	: _cam{fovy, aspect, near, far}, _move{_cam, w}, _look{_cam, w}
{}

template <typename Window>
void free_camera<Window>::input(float dt)
{
	_move.input(dt);
	_look.input(dt);
}

}  // gl
