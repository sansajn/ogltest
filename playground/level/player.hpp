// implementacia hraca
#pragma once
#include "player.hpp"
#include <glm/vec3.hpp>
#include "camera.hpp"
#include "controllers.hpp"
#include "window.hpp"

class fps_move  //!< wsad move in xz plane
{
public:
	fps_move(gl::camera & c, ui::glut_pool_window & w, float speed = 0.1);
	void input(float dt);
	void speed(float v) {_speed = v;}
	void controls(char forward, char backward, char left, char right);

private:
	enum class key {forward, backward, left, right, key_count};
	gl::camera & _cam;
	ui::glut_pool_window & _wnd;
	float _speed;
	char _controls[(int)key::key_count];
};

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
	player(ui::glut_pool_window & w);
	void position(glm::vec3 const & p);
	void view_parameters(view const & v);  // TODO: view properties
	gl::camera & get_camera() {return _cam;}
	gl::camera const & get_camera() const {return _cam;}
	glm::vec3 const & prev_position() const {return _prev_pos;}

	void input(float dt);
	void update(float dt);

private:
	gl::camera _cam;
	ui::glut_pool_window * _wnd;
	gl::free_look<ui::glut_pool_window> _look;
	fps_move _move;
	glm::vec3 _prev_pos;
};
