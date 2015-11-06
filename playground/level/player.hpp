// implementacia hraca
#pragma once
#include "player.hpp"
#include <glm/vec3.hpp>
#include "camera.hpp"
#include "controllers.hpp"
#include "window.hpp"
#include "physics/physics.hpp"

using namespace phys;

class fps_move  //!< wsad move in xz plane
{
public:
	fps_move() {}
	void init(ui::glut_pool_window::user_input * in, btRigidBody * body, float velocity = 2);
	void input(float dt);
	void controls(char forward, char backward, char left, char right);

private:
	enum class key {forward, backward, left, right, key_count};
	btRigidBody * _body;
	ui::glut_pool_window::user_input * _in;
	float _velocity;
	char _controls[(int)key::key_count+1];
};

class fps_look
{
public:
	fps_look() {}
	void init(ui::glut_pool_window * window, btRigidBody * body, float velocity = 1);
	void input(float dt);

private:
	ui::glut_pool_window * _window;
	btRigidBody * _body;
	float _velocity;
	bool _enabled = false;
};

class fps_player
{
public:
	fps_player() {}
	void init(glm::vec3 const & position, float fovy, float aspect_ratio, float near, float far, ui::glut_pool_window * window);
	gl::camera & get_camera() {return _cam;}
	void link_with(rigid_body_world & world, int mark = -1);
	btRigidBody * body() const {return _collision.native();}
	void input(float dt);
	void update(float dt);
//	render();

private:
	gl::camera _cam;
	body_object _collision;
	ui::glut_pool_window * _window;
	fps_look _look;
	fps_move _move;
};
