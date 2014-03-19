#pragma once
#include <map>
#include <string>
#include <utility>

namespace gl {

/*! Window wrapper.
\code
glut_window w;
w.start();
\endcode */
class glut_window
{
public:

	class parameters
	{
	public:
		parameters();
		std::string const & name() const {return _name;}
		parameters & size(int w, int h) {_size = std::make_pair(w, h); return *this;}
		std::pair<int, int> const & size() const {return _size;}
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

	glut_window(parameters const & params);
	virtual void start();

	virtual void display();
	virtual void reshape(int w, int h) {}
	virtual void idle();
	virtual void mouse_click(int button, int state, int x, int y) {}
	virtual void mouse_motion(int x, int y) {}
	virtual void mouse_passive_motion(int x, int y) {}
	virtual void mouse_wheel(int wheel, int direction, int x, int y) {}
	virtual void key_typed(unsigned char key, int x, int y) {}
	virtual void key_released(unsigned char key, int x, int y) {}
	virtual void special_key(int key, int x, int y) {}
	virtual void special_key_released(int key, int x, int y) {}

private:
	// see freeGlut for further documentation
	static void display_func();
	static void reshape_func(int w, int h);
	static void idle_func();
	static void mouse_func(int button, int state, int x, int y);
	static void motion_func(int x, int y);
	static void passive_motion_func(int x, int y);
	static void wheel_func(int wheel, int direction, int x, int y);
	static void keyboard_func(unsigned char key, int x, int y);
	static void keyboard_up_func(unsigned char key, int x, int y);
	static void special_func(int key, int x, int y);
	static void special_up_func(int key, int x, int y);

	int _id;

	static std::map<int, glut_window *> _windows;
};

}  // gl
