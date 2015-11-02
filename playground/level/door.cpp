#include "door.hpp"
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include "mechanics.hpp"

using glm::mat4;

door::door(btVector3 const & position, type orientation, gl::mesh const & m, texture2d & diff_tex)
	: _mesh{&m}, _diff_tex{&diff_tex}, _orient{orientation}
{
	_closed_pos = position + btVector3{.5, .5, -.5};

	btQuaternion rot = (_orient == type::vertical) ? btQuaternion{btVector3{0,1,0}, SIMD_HALF_PI} : btQuaternion{0,0,0,1};
	_collision = physics_object{make_box_shape(btVector3{.5, .5, .1}), 1000, _closed_pos, rot};  // TODO: make shape shared
	_collision.body()->setLinearFactor(btVector3{0,0,0});  // closed doors don't move
	_collision.body()->setAngularFactor(0);

	_state.init(this);
}

void door::update(float dt)
{
	_state.update(dt);
}

void door::render(shader::program & p, glm::mat4 const & world_to_screen)
{
	mat4 M = glm_cast(_collision.body()->getWorldTransform());
	p.uniform_variable("local_to_screen", world_to_screen * M);
	_diff_tex->bind(0);
	p.uniform_variable("s", 0);
	_mesh->render();
}

void door::link_with(rigid_body_world & world)
{
	world.world()->addRigidBody(_collision.body(), colgroup_door, ~colgroup_wall);  // dvere nekoliduju zo stenami
}

void door::open()
{
	_state.enter_openning_sequence();
}

btVector3 door::open_position() const
{
	return _closed_pos + (_orient == type::vertical ? btVector3{0, 0, -.9} : btVector3{-.9, 0, 0});
}

btVector3 const & door::closed_position() const
{
	return _closed_pos;
}

door::type door::orientation() const
{
	return _orient;
}


void door_openning::enter()
{
	btRigidBody * body = _door->collision().body();

	btVector3 vel = _door->orientation() == door::type::vertical ?
		btVector3{0, 0, -1.0f/DURATION} : btVector3{-1.0f/DURATION, 0, 0};
	body->setLinearVelocity(vel);

	if (!body->isActive())
		body->activate();
}

void door_openning::update(float dt)
{
	float open_min, cur;
	if (_door->orientation() == door::type::vertical)
	{
		open_min = _door->open_position().z();
		cur = _door->collision().position().z();
	}
	else
	{
		open_min = _door->open_position().x();
		cur = _door->collision().position().x();
	}

	if (cur <= open_min)
		owner->change_state((int)door_states::open);
}

void door_openning::exit()
{
	_door->collision().body()->setLinearVelocity(btVector3{0,0,0});  // zastav pohyb
}

void door_open::update(float dt)
{
	_t += dt;
	if (_t >= DURATION && _door->can_close())
		owner->change_state((int)door_states::closing);
}

void door_closing::enter()
{
	btRigidBody * body = _door->collision().body();

	btVector3 vel = _door->orientation() == door::type::vertical ?
		btVector3{0, 0, 1.0f/DURATION} : btVector3{1.0f/DURATION, 0, 0};
	body->setLinearVelocity(vel);

	if (!body->isActive())
		body->activate();
}

void door_closing::update(float dt)
{
	float closed_max, cur;
	if (_door->orientation() == door::type::vertical)
	{
		closed_max = _door->closed_position().z();
		cur = _door->collision().position().z();
	}
	else
	{
		closed_max = _door->closed_position().x();
		cur = _door->collision().position().x();
	}

	if (cur >= closed_max)
		owner->change_state((int)door_states::closed);
}

void door_closing::exit()
{
	btRigidBody * body = _door->collision().body();
	body->setLinearVelocity(btVector3{0,0,0});  // zastav pohyb
	body->setLinearFactor(btVector3{0,0,0});
}

void door_state_machine::init(door * d)
{
	_door = d;
	state_machine::init(&_closed);
}

void door_state_machine::enter_openning_sequence()
{
	if (current() == &_closed)
		change_state((int)door_states::openning);
}

void door_state_machine::change_state(int state_type_id)
{
	switch (state_type_id)
	{
		case (int)door_states::closed:
			_closed = door_closed{};
			state_machine::change_state(&_closed);
			break;

		case (int)door_states::openning:
			_openning = door_openning{_door};
			state_machine::change_state(&_openning);
			break;

		case (int)door_states::open:
			_open = door_open{_door};
			state_machine::change_state(&_open);
			break;

		case (int)door_states::closing:
			_closing = door_closing{_door};
			state_machine::change_state(&_closing);
			break;

		default:
			throw std::logic_error{"bad cast"};
	}
}
