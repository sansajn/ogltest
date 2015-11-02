// state machine
#include <stdexcept>
#include <iostream>
#include <glm/glm.hpp>
#include "fsm.hpp"

using glm::vec3;
using glm::mix;

enum class door_states {closed, openning, open, closing};

struct door {
	enum class type {vertical, horizontal};
	door();
	~door() {delete _state;}
	void update(float dt) {_state->update(dt);}
	bool can_close() {return true;}
	void position(vec3 p) {}
	type orientation() {return type::vertical;}
	vec3 closed_position() {return vec3{};}
	void open() {_state->change_state((int)door_states::openning);}
	state_machine * _state;
};

void play_openning_sound_async() {}


struct door_closed : public state_machine::state
{
	void enter() override {
		std::cout << "door_closed::enter()" << std::endl;
	}

	void update(float dt) override {}

	void exit() override {
		std::cout << "door_closed::exit()" << std::endl;
	}
};

class door_openning : public state_machine::state
{
public:
	door_openning() {}
	door_openning(door * d) : _door{d} {}

	void enter() override {
		play_openning_sound_async();
		std::cout << "door_openning::enter()" << std::endl;
	}

	void update(float dt) override
	{
		_t += dt;
		vec3 closed_pos = _door->closed_position();
		vec3 opened_pos = closed_pos +
			(_door->orientation() == door::type::vertical ? vec3{0,0,-1} : vec3{-1,0,0});
		_door->position(mix(closed_pos, opened_pos, _t/DURATION));

		if (_t >= DURATION)
			owner->change_state((int)door_states::open);
	}

	void exit() {
		std::cout << "door_openning::exit()" << std::endl;
	}

private:
	float DURATION = 1.0f;  // TODO: const
	float _t = 0;
	door * _door;
};

class door_open : public state_machine::state
{
public:
	door_open() {}
	door_open(door * d) : _door{d} {}

	void enter() {
		std::cout << "door_open::enter()" << std::endl;
	}

	void update(float dt) override
	{
		_t += dt;
		if (_t >= DURATION && _door->can_close())
			owner->change_state((int)door_states::closing);
	}

	void exit() {
		std::cout << "door_open::exit()" << std::endl;
	}

private:
	float DURATION = 3.0f;  // TODO: const
	float _t = 0;
	door * _door;
};

struct door_closing : public state_machine::state
{
	void enter() {
		std::cout << "door_closing::enter()" << std::endl;
	}

	void update(float dt) override {
		_t += dt;
		if (_t >= DURATION)
			owner->change_state((int)door_states::closed);
	}

	void exit() {
		std::cout << "door_closing::exit()" << std::endl;
	}

private:
	float DURATION = 1.0f;
	float _t = 0;
};

class door_state_machine : public state_machine
{
public:
	door_state_machine(door * d) : _door{d} {
		init(&_closed);
	}

	void change_state(int state_type_id) override
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
				_closing = door_closing{};
				state_machine::change_state(&_closing);
				break;

			default:
				throw std::logic_error{"bad cast"};
		}
	}

private:
	door_closed _closed;
	door_openning _openning;
	door_open _open;
	door_closing _closing;

	door * _door;
};

door::door()
	: _state{new door_state_machine{this}}
{}


int main(int argc, char * argv[])
{
	door d;
	d.open();  // open door as an user ation
	while (true)
		d.update(1.0/60.0);

	return 0;
}
