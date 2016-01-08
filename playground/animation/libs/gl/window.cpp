#include "gl/window.hpp"
#include <utility>

namespace ui {

using std::make_pair;

window_layer::parameters & window_layer::parameters::geometry(unsigned w, unsigned h)
{
	_w = w;
	_h = h;
	return *this;
}

window_layer::parameters & window_layer::parameters::name(std::string const & s)
{
	_name = s;
	return *this;
}

window_layer::parameters & window_layer::parameters::debug(bool d)
{
	_debug = d;
	return *this;
}

window_layer::parameters & window_layer::parameters::version(int major, int minor)
{
	_version = make_pair(major, minor);
	return *this;
}


window_layer::parameters::parameters()
{
	_w = 800;
	_h = 600;
	_name = "window";
	_debug = true;
	_version = make_pair(0, 0);
}

//#include <pthread.h>
//void junk()  // riesi bug v linuxe, ked pri spusteni binarky system zahlasi 'Inconsistency detected by ld.so: dl-version.c: 224: _dl_check_map_versions: Assertion `needed != ((void *)0)' failed!'
//{
//	int i;
//	i=pthread_getconcurrency();
//}

}   // ui
