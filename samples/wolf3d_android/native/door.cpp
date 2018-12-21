#include "door.hpp"
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include "gles2/model_gles2.hpp"
#include "mechanics.hpp"
#include "sound.hpp"
#include "resource.hpp"

using glm::mat4;
using gles2::texture2d;

std::string const door_object::open_sound_id = "sound/door.ogg";


door_object::door_object(btVector3 const & position, type orientation, gles2::mesh const & m, texture2d & diff_tex)
	: _mesh{&m}, _diff_tex{&diff_tex}, _orient{orientation}
{
	_closed_pos = position + btVector3{.5, .5, -.5};

	btQuaternion rot = (_orient == type::vertical) ? btQuaternion{btVector3{0,1,0}, SIMD_HALF_PI} : btQuaternion{0,0,0,1};
	_collision = body_object{make_box_shape(btVector3{.5, .5, .1}), 1000, _closed_pos, rot};  // TODO: make shape shared
	_collision.native()->setLinearFactor(btVector3{0,0,0});  // closed doors don't move
	_collision.native()->setAngularFactor(0);
}

void door_object::update(float dt)
{
	_state.update(dt, this);
}

void door_object::render(gles2::shader::program & p, glm::mat4 const & world_to_screen)
{
	mat4 M = glm_cast(_collision.transform());
	p.uniform_variable("local_to_screen", world_to_screen * M);
	_diff_tex->bind(0);
	p.uniform_variable("s", 0);
	_mesh->render();
}

void door_object::link_with(rigid_body_world & world, int mask)
{
	world.native()->addRigidBody(_collision.native(), colgroup_door, ~colgroup_wall);  // dvere nekoliduju zo stenami
	if (mask != -1)
		_collision.native()->setUserIndex(mask);
}

void door_object::open()
{
	_state.enter_open_sequence();
}

btVector3 door_object::open_position() const
{
	return _closed_pos + (_orient == type::vertical ? btVector3{0, 0, -.9} : btVector3{-.9, 0, 0});
}

btVector3 const & door_object::closed_position() const
{
	return _closed_pos;
}

door_object::type door_object::orientation() const
{
	return _orient;
}


void door_opening::enter(door_object * d)
{
	btRigidBody * body = d->collision().native();

	btVector3 vel = (d->orientation() == door_object::type::vertical) ?
		btVector3{0, 0, -1.0f/DURATION} : btVector3{-1.0f/DURATION, 0, 0};

	body->setLinearVelocity(vel);
	body->activate();

	al::device::ref().play_effect(path_manager::ref().translate_path(door_object::open_sound_id));
}

door_states door_opening::update(float dt, door_object * d)
{
	float open_min, cur;
	if (d->orientation() == door_object::type::vertical)
	{
		open_min = d->open_position().z();
		cur = d->collision().position().z();
	}
	else
	{
		open_min = d->open_position().x();
		cur = d->collision().position().x();
	}

	if (cur <= open_min)
		return door_states::open;

	return door_states::invalid;
}

void door_opening::exit(door_object * d)
{
	btRigidBody * body = d->collision().native();
	body->setLinearVelocity(btVector3{0,0,0});  // zastav pohyb
}

void door_open::enter(door_object * d)
{
	_t = 0;
}

door_states door_open::update(float dt, door_object * d)
{
	_t += dt;
	if (_t >= DURATION && d->can_close())
		return door_states::closing;

	return door_states::invalid;
}

void door_closing::enter(door_object * d)
{
	btRigidBody * body = d->collision().native();

	btVector3 vel = (d->orientation() == door_object::type::vertical) ?
		btVector3{0, 0, 1.0f/DURATION} : btVector3{1.0f/DURATION, 0, 0};
	body->setLinearVelocity(vel);
	body->activate();

	al::device::ref().play_effect(path_manager::ref().translate_path(door_object::open_sound_id));
}

door_states door_closing::update(float dt, door_object * d)
{
	float closed_max, cur;
	if (d->orientation() == door_object::type::vertical)
	{
		closed_max = d->closed_position().z();
		cur = d->collision().position().z();
	}
	else
	{
		closed_max = d->closed_position().x();
		cur = d->collision().position().x();
	}

	if (cur >= closed_max)
		return door_states::close;

	return door_states::invalid;  // TODO: namiesto invalid vracaj closing
}

void door_closing::exit(door_object * d)
{
	btRigidBody * body = d->collision().native();
	body->setLinearVelocity(btVector3{0,0,0});  // zastav pohyb
}


door_state_machine::door_state_machine() : state_machine{door_states::close}
{
	fill_states();
}

void door_state_machine::enter_open_sequence()
{
	if (current_state() == door_states::close || current_state() == door_states::invalid)  // TODO: tuto dvojznacnost treba poriesit (pociatocny stav)
		enqueue_state(door_states::opening);
}

state & door_state_machine::to_ref(state_descriptor s)
{
	return *_states[(int)s];
}

void door_state_machine::fill_states()
{
	_states[(int)door_states::close] = &_closed;
	_states[(int)door_states::opening] = &_opening;
	_states[(int)door_states::open] = &_opened;
	_states[(int)door_states::closing] = &_closing;
}
