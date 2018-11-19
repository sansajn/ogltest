#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "gl/controllers.hpp"

namespace gl { namespace glut {

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

	}  // glut

}  // gl
