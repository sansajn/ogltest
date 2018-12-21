// implementacia hraca
#pragma once
#include <glm/vec3.hpp>
#include "gl/camera.hpp"
#include "gles2/animation_gles2.hpp"
#include "gles2/touch_joystick_gles2.hpp"
#include "game.hpp"
#include "fsm.hpp"

//class fps_look
//{
//public:
//	fps_look() {}
//	void init(ui::glut_pool_window * window, btRigidBody * body, float velocity = 1);
//	void input(float dt);

//private:
//	ui::glut_pool_window * _window;
//	btRigidBody * _body;
//	float _velocity;
//	bool _enabled = false;
//};


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

/*! hrac je preprezentovany boxom levitujucim nad zemou, tento box je zaroven aj collision boxom */
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

	player_object();
	void init(glm::vec3 const & position, float aspect_ratio);
	gl::camera & get_camera() {return _cam;}
	void link_with(phys::rigid_body_world & world, int mark = -1);
	btRigidBody * body() const {return _collision.native();}
	void input(float dt);
	void update(float dt);
	void render(gles2::shader::program & prog);
	btTransform const & transform() const override {return _collision.transform();}
	void fire();
	void damage(unsigned amount);
	unsigned health() const {return _health;}
	void heal(unsigned amount);
	btVector3 const & velocity() const;  //!< smer a rychlost pohybu hraca
	void velocity(btVector3 const & v);

	// low level
	std::vector<glm::mat4> const & skeleton() const {return _mdl.skeleton();}
	gles2::animated_model & get_model() {return _mdl;}

private:
	gles2::animated_model _mdl;
	player_state_machine _state;

	gl::camera _cam;
	phys::body_object _collision;
	unsigned _health = 100;
};


class crosshair_object
{
public:
	crosshair_object();
	void render(gles2::shader::program & p, glm::mat4 const & local_to_screen);

private:
	gles2::mesh _quad;
	gles2::texture2d _tex;
};
