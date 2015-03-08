#include "fps.hpp"
#include <GL/freeglut.h>

namespace ui {

fps_window::fps_window()
{
	keyb_init();
	mouse_init();
}

void fps_window::start()
{
	unsigned t_last = glutGet(GLUT_ELAPSED_TIME);

	while (true)
	{
		unsigned t = glutGet(GLUT_ELAPSED_TIME);
		float dt = float(t_last - t)/1000.0;  // in sec
		t_last = t;

		glutMainLoopEvent();
		if (_closed)
			break;

		input();
		update(dt);
		keyb_update();
		mouse_update();
		display();
	}
}

void fps_window::close()
{
	_closed = true;
}

bool fps_window::key(unsigned char c) const
{
	return _keys[c];
}

bool fps_window::key_up(unsigned char c) const
{
	return _keys_up[c];
}

bool fps_window::mouse(button b) const
{
	return _mouse_buttons[int(b)];
}

bool fps_window::mouse_up(button b) const
{
	return _mouse_buttons_up[int(b)];
}

void fps_window::mouse_passive_motion(int x, int y)
{
	_mouse_pos = glm::ivec2(x,y);
}

void fps_window::mouse_click(button b, state s, modifier m, int x, int y)
{
	if (s == state::down)
		_mouse_buttons[int(b)] = true;
	else
	{
		_mouse_buttons[int(b)] = false;
		_mouse_buttons_up[int(b)] = true;
	}
}

void fps_window::key_typed(unsigned char c, modifier m, int x, int y)
{
	_keys[c] = true;
}

void fps_window::key_released(unsigned char c, modifier m, int x, int y)
{
	_keys[c] = false;
	_keys_up[c] = true;
}

void fps_window::keyb_update()
{
	for (bool & k : _keys_up)
		k = false;
}

void fps_window::mouse_update()
{
	for (bool & b : _mouse_buttons_up)
		b = false;
}

void fps_window::keyb_init()
{
	for (int i = 0; i < NUM_KEYS; ++i)
		_keys[i] = _keys_up[i] = false;
}

void fps_window::mouse_init()
{
	_mouse_pos = glm::vec2(0,0);
	for (int i = 0; i < int(button::number_of_buttons); ++i)
		_mouse_buttons[i] = _mouse_buttons_up[i] = false;
}

}  // ui

void free_look::input()
{
	if (_wnd.mouse(ui::event_handler::button::left) && !_enabled)
	{
		_enabled = true;
		glutSetCursor(GLUT_CURSOR_NONE);
	}

	if (_wnd.key(27) && _enabled)  // esc
	{
		_enabled = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}

	if (!_enabled)
		return;

	float const angular_movement = 0.1f;

	glm::ivec2 center = _wnd.center();
	glm::ivec2 delta = _wnd.mouse_position() - center;

	if (delta.x != 0)
	{
		float angle = angular_movement * delta.x;
		_cam.rotation = glm::normalize(glm::angleAxis(-angle, glm::vec3(0,1,0)) * _cam.rotation);
	}

	if (delta.y != 0)
	{
		float angle = angular_movement * delta.y;
		_cam.rotation = glm::normalize(glm::angleAxis(-angle, _cam.right()) * _cam.rotation);
	}

	if (delta.x != 0 || delta.y != 0)
		glutWarpPointer(center.x, center.y);
}

void free_move::input()
{
	float const movement = 0.1f;

	if (_wnd.key('a'))
		_cam.position -= _cam.right() * movement;

	if (_wnd.key('d'))
		_cam.position += _cam.right() * movement;

	if (_wnd.key('w'))
		_cam.position -= _cam.forward() * movement;

	if (_wnd.key('s'))
		_cam.position += _cam.forward() * movement;

	if (_wnd.key('z'))
		_cam.position += glm::vec3(0, -1, 0) * movement;

	if (_wnd.key('x'))
		_cam.position += glm::vec3(0, 1, 0) * movement;
}
