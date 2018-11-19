#pragma once
#include "glut_free_look.hpp"

namespace gl { namespace glut {

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
	glut::free_look<Window> _look;
};

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

	}  // glut

}  // gl
