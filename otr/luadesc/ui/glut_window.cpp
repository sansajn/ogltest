#include "glut_window.hpp"
#include <cassert>
#include <chrono>
#include <GL/freeglut.h>

static event_handler::key tospecial(int k);
static event_handler::modifier tomodifier(int m);
static double now_in_ms();

std::map<int, glut_window *> glut_window::_windows;


glut_window::glut_window() : glut_window(parameters())
{}

glut_window::glut_window(parameters const & params)
{
	_t = now_in_ms();
	_dt = 0.0;

	if (_windows.size() == 0)
	{
		int argc = 1;
		char * argv[1] = {(char *)"glut_window"};
		glutInit(&argc, argv);
	}

	if (params.version() != std::make_pair(-1, -1))
		glutInitContextVersion(params.version().first, params.version().second);

	glutInitContextFlags(params.debug() ? GLUT_DEBUG : 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowSize(params.size().first, params.size().second);
	_id = glutCreateWindow(params.name().c_str());

	_windows[_id] = this;

	glutDisplayFunc(display_func);
	glutReshapeFunc(reshape_func);
	glutIdleFunc(idle_func);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);
	glutKeyboardFunc(keyboard_func);
	glutKeyboardUpFunc(keyboard_up_func);
	glutSpecialFunc(special_func);
	glutSpecialUpFunc(special_up_func);

//	Event glutMouseWheelFunc() is not worrking in free-glut 2.8.1, see
//	glut_window::mouse_func() for workaround and glut_window::wheel_func() for
//	wheel event implementation.
//	glutMouseWheelFunc(wheel_func);

	window::glew_init();
}

void glut_window::start()
{
	glutMainLoop();
}

void glut_window::display(double t, double dt)
{
	glutSwapBuffers();
	_t = now_in_ms();
	_dt = _t - t;
}

void glut_window::idle()
{
	glutPostRedisplay();
}

void glut_window::display_func()
{
	glut_window * w = _windows[glutGetWindow()];
	w->display(w->_t, w->_dt);
}

void glut_window::reshape_func(int w, int h)
{
	_windows[glutGetWindow()]->reshape(w, h);
}

void glut_window::idle_func()
{
	_windows[glutGetWindow()]->idle();
}

void glut_window::mouse_func(int mouse_btn, int btn_state, int x, int y)
{
	modifier m = tomodifier(glutGetModifiers());
	glut_window * wnd = _windows[glutGetWindow()];
	if (mouse_btn == int(button::wheel_up) || mouse_btn == int(button::wheel_down))
	{
		if (btn_state == int(state::down))
		{
			wnd->mouse_wheel(
				(mouse_btn == int(button::wheel_up) ? wheel::up : wheel::down), m, x, y);
		}
		// ignore mouse wheel up event
	}
	else
		wnd->mouse_click(button(mouse_btn), state(btn_state), m, x, y);
}

void glut_window::motion_func(int x, int y)
{
	_windows[glutGetWindow()]->mouse_motion(x, y);
}

void glut_window::passive_motion_func(int x, int y)
{
	_windows[glutGetWindow()]->mouse_passive_motion(x, y);
}

//void glut_window::wheel_func(int wheel_btn, int direction, int x, int y)
//{
//	_windows[glutGetWindow()]->mouse_wheel(wheel(wheel_btn), x, y);
//}

void glut_window::keyboard_func(unsigned char c, int x, int y)
{
	modifier m = tomodifier(glutGetModifiers());
	_windows[glutGetWindow()]->key_typed(c, m, x, y);
}

void glut_window::keyboard_up_func(unsigned char c, int x, int y)
{
	modifier m = tomodifier(glutGetModifiers());
	_windows[glutGetWindow()]->key_released(c, m, x, y);
}

void glut_window::special_func(int k, int x, int y)
{
	key special = tospecial(k);
	if (special != key::unknown)
	{
		modifier m = tomodifier(glutGetModifiers());
		_windows[glutGetWindow()]->special_key(special, m, x, y);
	}
}

void glut_window::special_up_func(int k, int x, int y)
{
	key special = tospecial(k);
	if (special != key::unknown)
	{
		modifier m = tomodifier(glutGetModifiers());
		_windows[glutGetWindow()]->special_key_released(special, m, x, y);
	}
}

event_handler::key tospecial(int k)
{
	switch (k)
	{
		case GLUT_KEY_LEFT:
			return event_handler::key::left;
		case GLUT_KEY_UP:
			return event_handler::key::up;
		case GLUT_KEY_RIGHT:
			return event_handler::key::right;
		case GLUT_KEY_DOWN:
			return event_handler::key::down;
		case GLUT_KEY_PAGE_UP:
			return event_handler::key::page_up;
		case GLUT_KEY_PAGE_DOWN:
			return event_handler::key::page_down;
		case GLUT_KEY_HOME:
			return event_handler::key::home;
		case GLUT_KEY_END:
			return event_handler::key::end;
		case GLUT_KEY_INSERT:
			return event_handler::key::insert;
	};

	if ((k > GLUT_KEY_F1-1) && k < (GLUT_KEY_F12+1))
		return event_handler::key(k-GLUT_KEY_F1+1);

	return event_handler::key::unknown;  // undocumented alt, ctrl and shift
}

event_handler::modifier tomodifier(int m)
{
	int mods = 0;
	if (m & GLUT_ACTIVE_SHIFT)
		mods |= event_handler::modifier::shift;
	if (m & GLUT_ACTIVE_CTRL)
		mods |= event_handler::modifier::ctrl;
	if (m & GLUT_ACTIVE_ALT)
		mods |= event_handler::modifier::alt;
	assert(mods == 0 && "unknown glut modifier");
	return event_handler::modifier(mods);
}

double now_in_ms()
{
	typedef std::chrono::high_resolution_clock clock;
	clock::time_point tp = clock::now();
	return double(std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count());
}
