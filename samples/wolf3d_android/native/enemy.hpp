#pragma once
#include "game.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/mesh_gles2.hpp"
#include "gles2/texture_gles2.hpp"
#include "gles2/sprite_model_gles2.hpp"
#include "fsm.hpp"

class enemy_object;

enum class enemy_states {guard, attention, fight, chase, death, invalid};

using enemy_state_machine_state = default_state_machine_state<enemy_object, enemy_states>;

class enemy_guard : public enemy_state_machine_state
{
public:
	void enter(enemy_object * e) override;
	enemy_states update(float dt, enemy_object * e) override;
};

class enemy_attention : public enemy_state_machine_state
{
public:
	void enter(enemy_object * e) override;
	enemy_states update(float dt, enemy_object * e) override;

private:
	float _t = 0;
	float MAX_ATTENTION_TIME = 1.0f;
};

class enemy_fight : public enemy_state_machine_state
{
public:
	void enter(enemy_object * e) override;
	enemy_states update(float dt, enemy_object * e) override;

private:
	float _t = 0;
	float _fire_period = 1.0f/2.0f;
};

class enemy_chase : public enemy_state_machine_state
{
public:
	void enter(enemy_object * e) override;
	enemy_states update(float dt, enemy_object * e) override;
	void exit(enemy_object * e) override;

private:
	float _t = 0;
	float MAX_PLAYER_UNKNOWN_TIME = 5.0f;
};

class enemy_death : public enemy_state_machine_state
{
public:
	void enter(enemy_object * e) override;
	enemy_states update(float dt, enemy_object * e) override;
};

class enemy_state_machine
	: public state_machine<enemy_state_machine, enemy_object, enemy_states>
	, private boost::noncopyable
{
public:
	using state_descriptor = enemy_states;
	static state_descriptor const invalid_descriptor = enemy_states::invalid;

	enemy_state_machine();
	enemy_state_machine_state & to_ref(state_descriptor s);

private:
	void fill_states();

	enemy_guard _guard;
	enemy_attention _attention;
	enemy_fight _fight;
	enemy_chase _chase;
	enemy_death _death;
	enemy_state_machine_state * _states[5];
};


class enemy_object
	: public game_object, private boost::noncopyable
{
public:
	enemy_object(btVector3 const & position);  // TODO: model ako parameter
	~enemy_object();
	btTransform const & transform() const override;
	btCollisionObject * collision() const override;

	void update(float dt);
	void render(gles2::shader::program & p, glm::mat4 const & world_to_screen);
	void link_with(phys::rigid_body_world & world, int mask = -1);  // const
	btVector3 const & position() const {return transform().getOrigin();}
	void go(btVector3 const & velocity);
	bool see_player() const;
	float player_distance2() const;
	unsigned health() const {return _health;}
	void damage(unsigned amount);

	// low level api
	btVector3 last_known_player_pos;
	sprite_model & model() {return _model;}
	void remove_from_world();

	int _id = 0;  // debug

private:
	std::shared_ptr<btCollisionShape> shared_shape();

	phys::body_object _collision;
	glm::quat _rot;
	unsigned _health = 20;
	enemy_state_machine _state;
	sprite_model _model;

	static std::shared_ptr<btCollisionShape> _shape;
};
