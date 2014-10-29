#include "tweakbar_handler.hpp"

tweakbar_handler::tweakbar_handler(std::string const & name, bool active_flag)
	: _name(name), _active(active_flag), _need_update(false)
{}

void tweakbar_handler::display(bool & need_update)
{
	need_update = _need_update;
	_need_update = false;
}

void tweakbar_handler::reshape(int w, int h, bool & need_update)
{
	need_update = false;
}

void tweakbar_handler::idle(bool & need_update)
{
	need_update = false;
}

bool tweakbar_handler::mouse_click(event_handler::button b, event_handler::state s, int x, int y, bool & need_update)
{
	need_update = false;
	return false;
}

bool tweakbar_handler::mouse_motion(int x, int y, bool & need_update)
{
	need_update = false;
	return false;
}

bool tweakbar_handler::mouse_passive_motion(int x, int y, bool & need_update)
{
	need_update = false;
	return false;
}

bool tweakbar_handler::mouse_wheel(event_handler::wheel b, int x, int y, bool & need_update)
{
	need_update = false;
	return false;
}

bool tweakbar_handler::key_typed(unsigned char c, int x, int y, bool & need_update)
{
	need_update = false;
	return false;
}

bool tweakbar_handler::key_released(unsigned char c, int x, int y, bool & need_update)
{
	need_update = false;
	return false;
}

bool tweakbar_handler::special_key(event_handler::key k, int x, int y, bool & need_update)
{
	need_update = false;
	return false;
}

bool tweakbar_handler::special_key_released(event_handler::key k, int x, int y, bool & need_update)
{
	need_update = false;
	return false;
}
