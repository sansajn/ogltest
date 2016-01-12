#pragma once

//! implmentacia konecneho stavoveho stroja (finite state machine) pre herne objekty
template <typename Impl, typename Obj, typename Desc>
class state_machine
{
public:
	using state_descriptor = Desc;

	state_machine(state_descriptor s);
	void update(float dt, Obj * obj);
	void enqueue_state(state_descriptor s);
	state_descriptor current_state() const;

private:
	void change_state(state_descriptor s, Obj * obj);
	Impl * impl();

	state_descriptor _cur, _next;
};

template <typename T, typename D>
struct default_state_machine_state
{
	using value_type = T;
	using state_descriptor = D;

	virtual void enter(T * t) {}
	virtual state_descriptor update(float dt, T * t) = 0;
	virtual void exit(T * t) {}
};


template <typename Impl, typename Obj, typename Desc>
state_machine<Impl, Obj, Desc>::state_machine(state_descriptor s)
{
	_cur = s;
	_next = Impl::invalid_descriptor;
}

template <typename Impl, typename Obj, typename Desc>
void state_machine<Impl, Obj, Desc>::update(float dt, Obj * obj)
{
	if (_cur != _next)
	{
		if (_next != Impl::invalid_descriptor)
			change_state(_next, obj);
		else  // after creation
		{
			_next = _cur;
			impl()->to_ref(_cur).enter(obj);
		}
	}

	state_descriptor s = impl()->to_ref(_cur).update(dt, obj);
	if (s != Impl::invalid_descriptor)
		_next = s;
}

template <typename Impl, typename Obj, typename Desc>
void state_machine<Impl, Obj, Desc>::enqueue_state(state_descriptor s)
{
	_next = s;
}

template <typename Impl, typename Obj, typename Desc>
typename state_machine<Impl, Obj, Desc>::state_descriptor state_machine<Impl, Obj, Desc>::current_state() const
{
	return _cur;
}

template <typename Impl, typename Obj, typename Desc>
void state_machine<Impl, Obj, Desc>::change_state(state_descriptor s, Obj * obj)
{
	impl()->to_ref(_cur).exit(obj);
	_cur = s;
	impl()->to_ref(_cur).enter(obj);
}

template <typename Impl, typename Obj, typename Desc>
Impl * state_machine<Impl, Obj, Desc>::impl()
{
	return static_cast<Impl *>(this);
}
