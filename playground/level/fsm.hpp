//! implmentacia konecneho stavoveho stroja (finite state machine) pre herne objekty
#pragma once

//! TODO: vizitor koncept, podobne ako bgl, implementuj

class state_machine  //!< implmentacia konecneho stavoveho stroja pre herne objekty
{
public:
	struct state  //!< stav stavoveho stroja
	{
		virtual ~state() {}
		virtual void enter() {}
		virtual void update(float dt) {}
		virtual void exit() {}
		state_machine * owner = nullptr;
	};

	virtual ~state_machine() {}
	void init(state * s);
	virtual void update(float dt);
	virtual void change_state(state * s);
	virtual void change_state(int state_type_id) {}
	state * current() const {return _cur;}

private:
	state * _cur = nullptr;
};
