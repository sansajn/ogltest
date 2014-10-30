#pragma once
#include <string>
#include <utility>
#include <stdexcept>
#include "ui/event_handler.hpp"

struct window_error
	: public std::runtime_error
{
	window_error(std::string const & s) : std::runtime_error(s) {}
};

// TODO: volaj reshape po vytvoreni okna

/*! @ingroup ui */
class window : public event_handler
{
public:
	window();
	virtual ~window() {}

	virtual void start() = 0;
	void reshape(int w, int h) override;

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

protected:
	static void glew_init();  //!< must be called after create a valid rendering context
};  // window
