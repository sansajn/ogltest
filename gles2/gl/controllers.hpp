#pragma once
#include <functional>
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

template <typename PoolWindow>
free_move<PoolWindow>::free_move(camera & c, PoolWindow & w, float movement)
	: _cam(&c), _wnd(w), _movement(movement)
{
	controls('W', 'S', 'A', 'D');
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

	if (_wnd.in().key(_keys[int(key::up)]))
	{
		_cam->position += _cam->up() * _movement;
		moved = true;
	}

	if (_wnd.in().key(_keys[int(key::down)]))
	{
		_cam->position -= _cam->up() * _movement;
		moved = true;
	}

	if (_wnd.in().key(_keys[int(key::left)]))
	{
		_cam->position -= _cam->right() * _movement;
		moved = true;
	}

	if (_wnd.in().key(_keys[int(key::right)]))
	{
		_cam->position += _cam->right() * _movement;
		moved = true;
	}

	if (_wnd.in().key(_keys[int(key::forward)]))
	{
		_cam->position -= _cam->forward() * _movement;
		moved = true;
	}

	if (_wnd.in().key(_keys[int(key::backward)]))
	{
		_cam->position += _cam->forward() * _movement;
		moved = true;
	}

	if (moved && _move_callback)
		_move_callback();
}

}  // gl
