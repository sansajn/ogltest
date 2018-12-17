#pragma once
#include "gl/controllers.hpp"

namespace gl {

template <typename PoolWindow>  //!< \sa ui::window<ui::glut_pool_impl>
class free_look : public camera_controller
{
public:
	free_look(camera & c, PoolWindow & w, glm::vec3 const & up = glm::vec3(0,1,0));
	void input(float dt) override;
	void assoc_camera(camera & cam);  // TODO: naco je assoc_cam ak free_look nemozem vytvorit bez kamery ?

private:
	camera * _cam;
	PoolWindow & _wnd;
	glm::vec3 _up;
};

template <typename PoolWindow>
free_look<PoolWindow>::free_look(camera & c, PoolWindow & w, glm::vec3 const & up)
	: _wnd(w), _up(up)
{
	assoc_camera(c);
	_wnd.in().mode(ui::glfw3::user_input::input_mode::camera);
}

template <typename PoolWindow>
void free_look<PoolWindow>::input(float dt)
{
	using glm::vec2;
	using glm::vec3;

	assert(_cam);

	// rotation
	float angular_movement = 0.1f;  // rad/s
	vec3 const up = vec3{0,1,0};
	vec2 r = _wnd.in().mouse_position();

	if (r.x != 0.0f)
	{
		float angle = angular_movement * r.x * dt;
		_cam->rotation = normalize(angleAxis(-angle, up) * _cam->rotation);
	}

	if (r.y != 0.0f)
	{
		float angle = angular_movement * r.y * dt;
		_cam->rotation = normalize(angleAxis(-angle, _cam->right()) * _cam->rotation);
	}
}

template <typename PoolWindow>
void free_look<PoolWindow>::assoc_camera(camera & cam)
{
	_cam = &cam;
}

}  // gl
