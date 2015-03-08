#include "window.hpp"
#include <map>
#include <utility>
#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>

using std::map;
using std::make_pair;

namespace ui {

glut_window * active_window();
event_handler::modifier tomodifier(int m);
event_handler::key tospecial(int k);

// see freeGlut for further documentation
void display_func();
void reshape_func(int w, int h);
void idle_func();
void close_func();
void mouse_func(int button, int state, int x, int y);
void motion_func(int x, int y);
void passive_motion_func(int x, int y);
void keyboard_func(unsigned char key, int x, int y);
void keyboard_up_func(unsigned char key, int x, int y);
void special_func(int key, int x, int y);
void special_up_func(int key, int x, int y);


void window::reshape(int w, int h)
{
	_w = w;
	_h = h;
	glViewport(0, 0, w, h);
}

void window::bind_as_render_target()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, _w, _h);
}

void window::glew_init()
{
	assert(glGetError() == 0);
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
		throw window_error("GLEW initialization failed");
	glGetError();  // eat error
}


map<int, glut_window *> glut_windows;

window::parameters::parameters()
{
	_name = "window";
	_version = std::make_pair(-1, -1);  // default version
	_debug = false;
	_w = 800;
	_h = 600;
}

glut_window::glut_window()	: glut_window(parameters())
{}

glut_window::glut_window(parameters const & p)
{
	if (glut_windows.empty())
	{
		int argc = 1;
		char * argv[1] = {(char *)"dummy"};
		glutInit(&argc, argv);
	}
	
	if (p.version() != make_pair(-1, -1))
		glutInitContextVersion(p.version().first, p.version().second);
	
	if (p.debug())
		glutInitContextFlags(GLUT_DEBUG);
	
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	_w = p.width();
	_h = p.height();
	glutInitWindowSize(_w, _h);
	
	_wid = glutCreateWindow(p.name().c_str());
	glut_windows[_wid] = this;

	glutDisplayFunc(display_func);
	glutReshapeFunc(reshape_func);
	glutIdleFunc(idle_func);
	glutCloseFunc(close_func);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);
	glutKeyboardFunc(keyboard_func);
	glutKeyboardUpFunc(keyboard_up_func);
	glutSpecialFunc(special_func);
	glutSpecialUpFunc(special_up_func);

	window::glew_init();
}

glut_window::~glut_window()
{
	if (glutGetWindow())  // ak som klikol na x (okno je uz neplatne)
		glutDestroyWindow(_wid);
	glut_windows.erase(_wid);
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

void display_func()
{
	glut_window * w = active_window();
	w->display();
}

void reshape_func(int w, int h)
{
	active_window()->reshape(w, h);
}

void idle_func()
{
	active_window()->idle();
}

void close_func()
{
	active_window()->close();
}

void mouse_func(int mouse_btn, int btn_state, int x, int y)
{
	using eh = event_handler;

	glut_window * w = active_window();
	eh::modifier m = tomodifier(glutGetModifiers());

	if (mouse_btn == 3 || mouse_btn == 4)  // 3:button::wheel_up, 4:button::wheel_down
	{
		if (btn_state == GLUT_DOWN)
			w->mouse_wheel((mouse_btn == 3 ? eh::wheel::up : eh::wheel::down), m, x, y);
	}
	else
		w->mouse_click(eh::button(mouse_btn), eh::state(btn_state), m, x, y);
}

void motion_func(int x, int y)
{
	active_window()->mouse_motion(x, y);
}

void passive_motion_func(int x, int y)
{
	active_window()->mouse_passive_motion(x, y);
}

void keyboard_func(unsigned char c, int x, int y)
{
	event_handler::modifier m = tomodifier(glutGetModifiers());
	active_window()->key_typed(c, m, x, y);
}

void keyboard_up_func(unsigned char c, int x, int y)
{
	event_handler::modifier m = tomodifier(glutGetModifiers());
	active_window()->key_released(c, m, x, y);
}

void special_func(int k, int x, int y)
{
	using eh = event_handler;

	eh::key special = tospecial(k);
	if (special != eh::key::unknown)
	{
		eh::modifier m = tomodifier(glutGetModifiers());
		active_window()->special_key(special, m, x, y);
	}
}

void special_up_func(int k, int x, int y)
{
	using eh = event_handler;

	eh::key special = tospecial(k);
	if (special != eh::key::unknown)
	{
		eh::modifier m = tomodifier(glutGetModifiers());
		active_window()->special_key_released(special, m, x, y);
	}
}

glut_window * active_window() 
{
	return glut_windows[glutGetWindow()];
}

event_handler::key tospecial(int k)
{
	using eh = event_handler;

	switch (k)
	{
		case GLUT_KEY_LEFT:
			return eh::key::left;
		case GLUT_KEY_UP:
			return eh::key::up;
		case GLUT_KEY_RIGHT:
			return eh::key::right;
		case GLUT_KEY_DOWN:
			return eh::key::down;
		case GLUT_KEY_PAGE_UP:
			return eh::key::page_up;
		case GLUT_KEY_PAGE_DOWN:
			return eh::key::page_down;
		case GLUT_KEY_HOME:
			return eh::key::home;
		case GLUT_KEY_END:
			return eh::key::end;
		case GLUT_KEY_INSERT:
			return eh::key::insert;
	};

	if ((k > GLUT_KEY_F1-1) && k < (GLUT_KEY_F12+1))
		return eh::key(k-GLUT_KEY_F1+1);

	return eh::key::unknown;  // undocumented alt, ctrl and shift
}

event_handler::modifier tomodifier(int m)
{
	using eh = event_handler;

	int mods = 0;
	if (m & GLUT_ACTIVE_SHIFT)
		mods |= eh::modifier::shift;
	if (m & GLUT_ACTIVE_CTRL)
		mods |= eh::modifier::ctrl;
	if (m & GLUT_ACTIVE_ALT)
		mods |= eh::modifier::alt;
	assert(mods == 0 && "unknown glut modifier");

	return eh::modifier(mods);
}

};  // ui
