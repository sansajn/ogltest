//! implmentuje herny objekt dvere
#pragma once
#include <boost/noncopyable.hpp>
#include "gles2/mesh_gles2.hpp"
#include "gles2/texture_gles2.hpp"
#include "gles2/program_gles2.hpp"
#include "physics/physics.hpp"
#include "fsm.hpp"

using namespace phys;

enum class door_states {close, opening, open, closing, invalid};

class door_object;

using state = default_state_machine_state<door_object, door_states>;

struct door_close : public state
{
	door_states update(float dt, door_object *d) override {return door_states::invalid;}
};

class door_opening : public state
{
public:
	void enter(door_object * d) override;
	door_states update(float dt, door_object * d) override;
	void exit(door_object * d) override;

private:
	float DURATION = 1.4f;
};

class door_open : public state
{
public:
	void enter(door_object * d) override;
	door_states update(float dt, door_object * d) override;

private:
	float _t = 0;
	float DURATION = 3.0f;
};

class door_closing : public state
{
public:
	void enter(door_object * d) override;
	door_states update(float dt, door_object * d) override;
	void exit(door_object * d) override;

private:
	float DURATION = 1.4f;
};


class door_state_machine
	: public state_machine<door_state_machine, door_object, door_states>
	, private boost::noncopyable
{
public:
	using state_descriptor = door_states;
	static state_descriptor const invalid_descriptor = door_states::invalid;

	door_state_machine();
	void enter_open_sequence();
	state & to_ref(state_descriptor s);

private:
	void fill_states();

	door_close _closed;
	door_opening _opening;
	door_open _opened;
	door_closing _closing;
	state * _states[4];
};


class door_object : private boost::noncopyable
{
public:
	enum type {horizontal, vertical};

	door_object(btVector3 const & position, type orientation, gles2::mesh const & m, gles2::texture2d & diff_tex);
	void update(float dt);
	void render(gles2::shader::program & p, glm::mat4 const & world_to_screen);
	void link_with(rigid_body_world & world, int mask = -1);  // const
	void open();
	bool can_close() const {return true;}

	// low level
	body_object & collision() {return _collision;}
	btVector3 open_position() const;
	btVector3 const & closed_position() const;
	type orientation() const;

	static std::string const open_sound_id;

private:
	body_object _collision;
	door_state_machine _state;
	btVector3 _closed_pos;
	gles2::mesh const * _mesh;
	gles2::texture2d * _diff_tex;  // TODO: make const
	type _orient;
};
