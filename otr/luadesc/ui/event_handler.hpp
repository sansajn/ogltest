#pragma once
#include <string>

/*! \ingroup ui */
class event_handler
{
public:
	//! Mouse buttons.
	enum class button
	{
		left,  // 0
		middle,
		right,
		wheel_up,
		wheel_down
	};
	
	//! Mouse button states.
	enum class state
	{
		down,
		up
	};

	//! Mouse wheel event.
	enum class wheel
	{
		up,
		down
	};

	//! Modifier keys.
	enum modifier
	{
		shift = 1,
		ctrl = 2,
		alt = 4
	};

	//! Special keys.
	enum class key
	{
		caps_lock,  // 0
		f1,
		f2,
		f3,
		f4,
		f5,
		f6,
		f7,
		f8,
		f9,
		f10,
		f11,
		f12,
		print_screen,
		scroll_lock,
		pause,
		insert,
		home,
		page_up,
		unused1,  // there is a gap there in sdl
		end,
		page_down,
		right,
		left,
		down,
		up,
		unknown
	};

	std::string key_name(key const & k);
	std::string button_name(button const & b);

	virtual void display(double t, double dt) {}
	virtual void reshape(int w, int h) {}
	virtual void idle() {}
	virtual bool mouse_click(button b, state s, modifier m, int x, int y) {return false;}
	virtual bool mouse_motion(int x, int y) {return false;}
	virtual bool mouse_passive_motion(int x, int y) {return false;}  //!< \note no mouse buttons are pressed
	virtual bool mouse_wheel(wheel b, modifier m, int x, int y) {return false;}
	virtual bool key_typed(unsigned char c, modifier m, int x, int y) {return false;}
	virtual bool key_released(unsigned char c, modifier m, int x, int y) {return false;}
	virtual bool special_key(key k, modifier m, int x, int y) {return false;}
	virtual bool special_key_released(key k, modifier m, int x, int y) {return false;}
};  // event_handler
