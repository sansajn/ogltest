#pragma once
#include <string>
#include <stdexcept>

namespace ui {

struct window_error : public std::runtime_error
{
	window_error(std::string const & s) : std::runtime_error(s) {}
};

class event_handler
{
public:
	enum class button  //!< mouse buttons
	{
		left,
		middle,
		right,
		wheel_up,
		wheel_down,
		number_of_buttons
	};
	
	enum class state  //!< button states
	{
		down,
		up
	};
	
	enum class wheel
	{
		up,
		down
	};
	
	enum modifier
	{
		none = 0,
		shift = 1,
		ctrl = 2,
		alt = 4
	};
	
	enum class key  //!< special keys
	{
		caps_lock,
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
		end,
		page_down,
		right,
		left,
		down,
		up,
		unknown
	};
	
	virtual void display() {}
	virtual void reshape(int w, int h) {}
	virtual void idle() {}
	virtual void close() {}
	virtual void mouse_click(button b, state s, modifier m, int x, int y) {}
	virtual void mouse_motion(int x, int y) {}
	virtual void mouse_passive_motion(int x, int y) {}
	virtual void mouse_wheel(wheel w, modifier m, int x, int y) {}
	virtual void key_typed(unsigned char c, modifier m, int x, int y) {}
	virtual void key_released(unsigned char c, modifier m, int x, int y) {}
	virtual void special_key(key k, modifier m, int x, int y) {}
	virtual void special_key_released(key k, modifier m, int x, int y) {}
};  // event_handler

class window : public event_handler  //!< window abstraction
{
public:
	virtual ~window() {}
	virtual void start() = 0;
	void reshape(int w, int h) override;

	unsigned width() const {return _w;}
	unsigned height() const {return _h;}
	float aspect_ratio() const {return _w/float(_h);}  // TODO: aspect_ratio()

	void bind_as_render_target();
	
	class parameters 
	{
	public:
		using self = parameters;

		parameters();

		int width() const {return _w;}
		int height() const {return _h;}
		std::string const & name() const {return _name;}
		bool debug() const {return _debug;}
		std::pair<int, int> version() const {return _version;}
		
		self & size(int w, int h) {_w = w; _h = h; return *this;}
		self & name(std::string const & s) {_name = s; return *this;}
		self & debug(bool d) {_debug = d; return *this;}
		self & version(int major, int minor) {_version = std::make_pair(major, minor); return *this;}
		
	private:
		int _w, _h;
		std::string _name;
		bool _debug;
		std::pair<int, int> _version;  // (major, minor)
	};

protected:
	static void glew_init();  //!< \note volaj az po vytvoreni render kontextu
	unsigned _w, _h;  //!< window geometry
};  // window

class glut_window : public window  // glut window implementation
{
public:
	glut_window();
	glut_window(parameters const & p);
	~glut_window();
	
	void start() override;
	void display() override;
	void idle() override;

private:
	int _wid;  //!< window id
};

}  // ui
