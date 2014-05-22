#pragma once
#include <string>
#include <utility>
#include <stdexcept>
#include "event_handler.h"

namespace gl {

struct window_error
	: public std::runtime_error
{
	window_error(std::string const & s) : std::runtime_error(s) {}
};


class window
	: public event_handler
{
public:
	virtual void start() {}

	virtual void display() = 0;
	virtual void reshape(int w, int h) {}
	virtual void idle() {}
	virtual void mouse_click(button b, state s, int x, int y) {}
	virtual void mouse_motion(int x, int y) {}
	virtual void mouse_passive_motion(int x, int y) {}  //!< \note no mouse buttons are pressed
	virtual void mouse_wheel(wheel b, int x, int y) {}
	virtual void key_typed(unsigned char c, int x, int y) {}
	virtual void key_released(unsigned char c, int x, int y) {}
	virtual void special_key(key k, int x, int y) {}
	virtual void special_key_released(key k, int x, int y) {}

	class parameters
	{
	public:
		parameters();

		std::string const & name() const {return _name;}
		parameters & name(std::string const & s) {_name = s; return *this;}

		std::pair<int, int> const & size() const {return _size;}
		int width() const {return _size.first;}
		int height() const {return _size.second;}
		parameters & size(int w, int h) {_size = std::make_pair(w, h); return *this;}

		bool debug() const {return _debug;}
		parameters & debug(bool d) {_debug = d; return *this;}

		std::pair<int, int> const & version() const {return _version;}
		parameters & version(int major, int minor);

	private:
		std::pair<int, int> _size;  // (width, height)
		std::string _name;
		std::pair<int, int> _version;  // (major, minor)
		bool _debug;
	};
};  // window

};  // gl
