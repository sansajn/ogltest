#pragma once
#include <map>
#include <string>
#include <utility>
#include "ui/window.hpp"

/*! GLUT window wrapper.
\code
glut_window w;
w.start();
\endcode */
class glut_window : public window
{
public:
	glut_window();
	glut_window(parameters const & params);
	~glut_window() {}  // TODO: implement

	void start() override;
	void display(double t, double dt) override;
	void idle() override;

private:
	double _t;  //!< cas posledneho vykonania display() v ms
	double _dt;  //!< cas medzi dvoma poslednymi vykonaniami display() v ms

	// see freeGlut for further documentation
	static void display_func();
	static void reshape_func(int w, int h);
	static void idle_func();
	static void mouse_func(int button, int state, int x, int y);
	static void motion_func(int x, int y);
	static void passive_motion_func(int x, int y);
	static void keyboard_func(unsigned char key, int x, int y);
	static void keyboard_up_func(unsigned char key, int x, int y);
	static void special_func(int key, int x, int y);
	static void special_up_func(int key, int x, int y);

//	static void wheel_func(int wheel, int direction, int x, int y);

	int _id;

	static std::map<int, glut_window *> _windows;
};
