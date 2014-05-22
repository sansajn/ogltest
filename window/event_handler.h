#pragma once
#include <string>

namespace gl {

struct event_handler
{
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
	enum class modifier
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

};  // event_handler

}  // gl
