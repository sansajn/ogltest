#include <cassert>
#include <GL/freeglut.h>
#include "glut_window.h"

namespace gl {

static window::key tospecial(int k);

std::map<int, glut_window *> glut_window::_windows;


glut_window::glut_window(parameters const & params)
{
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
}

void glut_window::start()
{
	glutMainLoop();
}

void glut_window::display()
{
	glutSwapBuffers();
}

void glut_window::idle()
{
	glutPostRedisplay();
}

void glut_window::display_func()
{
	_windows[glutGetWindow()]->display();
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
	glut_window * wnd = _windows[glutGetWindow()];
	if (mouse_btn == int(button::wheel_up) || mouse_btn == int(button::wheel_down))
	{
		if (btn_state == int(state::down))
		{
			wnd->mouse_wheel(
				(mouse_btn == int(button::wheel_up) ? wheel::up : wheel::down), x, y);
		}
		// ignore mouse wheel up event
	}
	else
		wnd->mouse_click(button(mouse_btn), state(btn_state), x, y);
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
	_windows[glutGetWindow()]->key_typed(c, x, y);
}

void glut_window::keyboard_up_func(unsigned char c, int x, int y)
{
	_windows[glutGetWindow()]->key_released(c, x, y);
}

void glut_window::special_func(int k, int x, int y)
{
	key special = tospecial(k);
	if (special != key::unknown)
		_windows[glutGetWindow()]->special_key(special, x, y);
}

void glut_window::special_up_func(int k, int x, int y)
{
	key special = tospecial(k);
	if (special != key::unknown)
		_windows[glutGetWindow()]->special_key_released(special, x, y);
}

window::key tospecial(int k)
{
	switch (k)
	{
		case GLUT_KEY_LEFT:
			return window::key::left;
		case GLUT_KEY_UP:
			return window::key::up;
		case GLUT_KEY_RIGHT:
			return window::key::right;
		case GLUT_KEY_DOWN:
			return window::key::down;
		case GLUT_KEY_PAGE_UP:
			return window::key::page_up;
		case GLUT_KEY_PAGE_DOWN:
			return window::key::page_down;
		case GLUT_KEY_HOME:
			return window::key::home;
		case GLUT_KEY_END:
			return window::key::end;
		case GLUT_KEY_INSERT:
			return window::key::insert;
	};

	if ((k > GLUT_KEY_F1-1) && k < (GLUT_KEY_F12+1))
		return window::key(k-GLUT_KEY_F1+1);

	return window::key::unknown;  // undocumented alt, ctrl and shift
}

}  // gl
