// implementacia hraca
#pragma once
#include <glm/vec3.hpp>
#include "gl/controllers.hpp"
#include "gl/window.hpp"
#include "gl/program.hpp"
#include "game.hpp"
#include "model.hpp"
#include "fsm.hpp"

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

class fps_player : public game_object
{
public:
	fps_player() {}
	void init(glm::vec3 const & position, float fovy, float aspect_ratio, float near, float far, ui::glut_pool_window * window);
	gl::camera & get_camera() {return _cam;}
	void link_with(phys::rigid_body_world & world, int mark = -1);
	btRigidBody * body() const {return _collision.native();}
	void input(float dt);
	void update(float dt);
	btTransform const & transform() const override {return _collision.transform();}
//	render();

private:
	gl::camera _cam;
	phys::body_object _collision;
	ui::glut_pool_window * _window;
	fps_look _look;
	fps_move _move;
};


class player_object;

enum class player_states {idle, fire, invalid};

using player_state_machine_state = default_state_machine_state<player_object, player_states>;

class player_idle : public player_state_machine_state
{
public:
	void enter(player_object * p) override;
	player_states update(float dt, player_object * p) override;  // TODO: chcemmad defaultnu implementaciu (nikdy neukonci aktualny stav)
	void exit(player_object * p) override {}
};

class player_fire : public player_state_machine_state
{
public:
	void enter(player_object * p) override;
	player_states update(float dt, player_object * p) override;
	void exit(player_object * p) override {}

private:
	float _t = 0;
	float DURATION = .3f;
};

class player_state_machine
	: public state_machine<player_state_machine, player_object, player_states>
	, private boost::noncopyable
{
public:
	using state_descriptor = player_states;
	static state_descriptor const invalid_descriptor = player_states::invalid;

	player_state_machine();
	void enter_fire_sequence();
	player_state_machine_state & to_ref(state_descriptor s);  // TODO: to_state_ref()

private:
	void fill_states();

	player_idle _idle;
	player_fire _fire;
	player_state_machine_state * _states[2];
};


class player_object : public game_object
{
public:
	enum {  // animationas
		big_recoil_animation,
		charge_up_animation,
		fire_animation,
		fire2_animation,
		flashlight_animation,
		idle_animation,
		lower_animation,
		raise_animation
	};

	enum {  // sound effects
		fire1_sfx
	};

	void init(glm::vec3 const & position, float fovy, float aspect_ratio, float near, float far, ui::glut_pool_window * window);
	gl::camera & get_camera() {return _cam;}
	void link_with(phys::rigid_body_world & world, int mark = -1);
	btRigidBody * body() const {return _collision.native();}
	void input(float dt);
	void update(float dt);
	void render(shader::program & prog);
	btTransform const & transform() const override {return _collision.transform();}
	void fire();
	void damage(unsigned amount);
	unsigned health() const {return _health;}
	void heal(unsigned amount);

	// low level
	std::vector<glm::mat4> const & skeleton() const {return _mdl.skeleton();}
	animated_textured_model & get_model() {return _mdl;}

private:
	animated_textured_model _mdl;
	player_state_machine _state;

	gl::camera _cam;
	phys::body_object _collision;
	ui::glut_pool_window * _window;
	fps_look _look;
	fps_move _move;
	unsigned _health = 100;
};


class crosshair_object
{
public:
	crosshair_object();
	void render(shader::program & p, glm::mat4 const & local_to_screen);

private:
	gl::mesh _quad;
	texture2d _tex;
};
