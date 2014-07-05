#include "event_handler.h"

namespace gl {

std::string event_handler::key_name(key const & k)
{
	static std::string names[] = {
		"caps lock",
		"f1",
		"f2",
		"f3",
		"f4",
		"f5",
		"f6",
		"f7",
		"f8",
		"f9",
		"f10",
		"f11",
		"f12",
		"print screen",
		"scroll lock",
		"pause",
		"insert",
		"home",
		"page up",
		"unused1",
		"end",
		"page down",
		"right",
		"left",
		"down",
		"up",
		"unknown"
	};

	if (k < key::unknown)
		return names[int(k)];
	else
		return names[int(key::unknown)];
}

std::string event_handler::button_name(button const & b)
{
	static std::string names[] = {
		"left",
		"middle",
		"right",
		"wheel up",
		"wheel down"
	};

	if (b <= button::wheel_down)
		return names[int(b)];
	else
		return "unknown";
}

};
