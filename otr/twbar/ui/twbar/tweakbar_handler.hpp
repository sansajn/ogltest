#pragma once
#include <string>
#include <AntTweakBar.h>
#include "ui/event_handler.hpp"

class tweakbar_handler
{
public:
	tweakbar_handler(std::string const & name, bool active_flag);
	virtual ~tweakbar_handler() {}

	bool active() const {return _active;}
	virtual void active(bool val) {_active = val;}
	std::string const & name() const {return _name;}

	//! \param[out] need_update true if the twekbar must be updated
	virtual void display(bool & need_update);
	virtual void reshape(int w, int h, bool & need_update);
	virtual void idle(bool & need_update);
	virtual bool mouse_click(event_handler::button b, event_handler::state s, event_handler::modifier m, int x, int y, bool & need_update);
	virtual bool mouse_motion(int x, int y, bool & need_update);
	virtual bool mouse_passive_motion(int x, int y, bool & need_update);
	virtual bool mouse_wheel(event_handler::wheel b, event_handler::modifier m, int x, int y, bool & need_update);
	virtual bool key_typed(unsigned char c, event_handler::modifier m, int x, int y, bool & need_update);
	virtual bool key_released(unsigned char c, event_handler::modifier m, int x, int y, bool & need_update);
	virtual bool special_key(event_handler::key k, event_handler::modifier m, int x, int y, bool & need_update);
	virtual bool special_key_released(event_handler::key k, event_handler::modifier m, int x, int y, bool & need_update);

	virtual void update_bar(TwBar * bar) = 0;

private:
	std::string _name;
	bool _active;
	bool _need_update;
};
