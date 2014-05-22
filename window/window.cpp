#include "window.h"

namespace gl {

window::parameters::parameters()
{
	_size = std::make_pair(800, 600);
	_name = "window";
	_version = std::make_pair(4, 0);
	_debug = false;
}

window::parameters & window::parameters::version(int major, int minor)
{
	_version = std::make_pair(major, minor);
	return *this;
}

};  // gl
