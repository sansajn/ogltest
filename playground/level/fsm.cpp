#include "fsm.hpp"

void state_machine::init(state * s)
{
	_cur = s;
	_cur->owner = this;
	_cur->enter();
}

void state_machine::change_state(state * s)
{
	_cur->exit();
	_cur = s;
	_cur->owner = this;
	_cur->enter();
}

void state_machine::update(float dt)
{
	_cur->update(dt);
}
