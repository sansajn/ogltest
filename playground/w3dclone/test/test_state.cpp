// test stavoveho stroja (finite state machine)
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include "fsm.hpp"

using std::vector;
using glm::vec3;
using glm::mix;

enum class door_states {close, opening, open, closing, invalid};

struct door_object;

struct state
{
	virtual void enter(door_object *) {}
	virtual door_states update(float dt, door_object *) {return door_states::invalid;}
	virtual void exit(door_object *) {}
};

struct door_close : public state
{
	void enter(door_object *) override {std::cout << "door_close::enter()" << std::endl;}
	door_states update(float dt, door_object *) override {return door_states::invalid;}
	void exit(door_object *) override {std::cout << "door_close::exit()" << std::endl;}
};

class door_opening : public state
{
public:
	void enter(door_object *) override {std::cout << "door_opening::enter()" << std::endl;}
	door_states update(float dt, door_object * d) override;
	void exit(door_object *) override {std::cout << "door_opening::exit()" << std::endl;}

private:
	float DURATION = 1.0f;
	float _t = 0;
	door_object * _door;
};

class door_open : public state
{
public:
	void enter(door_object *) override {std::cout << "door_open::enter()" << std::endl;}
	door_states update(float dt, door_object *) override {return door_states::closing;}
	void exit(door_object *) override {std::cout << "door_open::exit()" << std::endl;}
};

struct door_closing : public state
{
	void enter(door_object *) override {std::cout << "door_closing::enter()" << std::endl;}
	door_states update(float dt, door_object *) override {return door_states::close;}
	void exit(door_object *) override {std::cout << "door_closing::exit()" << std::endl;}
};


class door_state_machine
	: public state_machine<door_state_machine, door_object, door_states>
{
public:
	using state_descriptor = door_states;
	static state_descriptor const invalid_descriptor = door_states::invalid;

	door_state_machine() : state_machine{door_states::close} {
		fill_states();
	}

	void enter_open_sequence() {
		if (current_state() == door_states::close || current_state() == door_states::invalid)  // TODO: tuto dvojznacnost treba poriesit (pociatocny stav)
			enqueue_state(door_states::opening);
	}

	state & to_ref(state_descriptor s) {return *_states[(int)s];}

private:
	void fill_states() {
		_states[(int)door_states::close] = &_closed;
		_states[(int)door_states::opening] = &_opening;
		_states[(int)door_states::open] = &_opened;
		_states[(int)door_states::closing] = &_closing;
	}

	door_close _closed;
	door_opening _opening;
	door_open _opened;
	door_closing _closing;
	state * _states[4];
};


struct door_object
{
	enum class type {vertical, horizontal};
	door_object() {}
	door_object(door_object && other) : _state{other._state} {}
	void update(float dt) {_state.update(dt, this);}
	bool can_close() {return true;}
	void position(vec3 p) {}
	type orientation() {return type::vertical;}
	vec3 closed_position() {return vec3{};}
	void open() {_state.enter_open_sequence();}
	door_object(door_object const &) = delete;
	door_object & operator=(door_object const &) = delete;
	door_state_machine _state;
};


door_states door_opening::update(float dt, door_object * d)
{
	_t += dt;
	vec3 closed_pos = _door->closed_position();
	vec3 opened_pos = closed_pos +
		(_door->orientation() == door_object::type::vertical ? vec3{0,0,-1} : vec3{-1,0,0});
	_door->position(mix(closed_pos, opened_pos, _t/DURATION));

	if (_t >= DURATION)
		return door_states::open;

	return door_states::invalid;
}


int main(int argc, char * argv[])
{
	vector<door_object> _doors;
	for (int i = 0; i < 3; ++i)
		_doors.push_back(door_object{});

	door_object & d = _doors.front();
	d.update(10);
	d.open();
	while (true)
		d.update(10);

	return 0;
}
