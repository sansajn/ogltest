#include "glut_window.h"
#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl {

std::map<int, glut_window *> glut_window::_windows;


glut_window::parameters::parameters()
{
	_size = std::make_pair(800, 600);
	_name = "glut_window";
	_version = std::make_pair(4, 0);
	_debug = false;
}

glut_window::parameters & glut_window::parameters::version(int major, int minor)
{
	_version = std::make_pair(major, minor);
	return *this;
}

glut_window::glut_window(parameters const & params)
{
	if (_windows.size() == 0)
	{
		int argc = 1;
		char * argv[1] = {(char *)"glut_window"};
		glutInit(&argc, argv);
	}

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
	glutMouseWheelFunc(wheel_func);
	glutKeyboardFunc(keyboard_func);
	glutKeyboardUpFunc(keyboard_up_func);
	glutSpecialFunc(special_func);
	glutSpecialUpFunc(special_up_func);

	assert(glGetError() == 0);
	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();  // eat error
}

void glut_window::start()
{
	glutMainLoop();
}

void glut_window::display()
{
	glutSwapBuffers();
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

void glut_window::mouse_func(int button, int state, int x, int y)
{
	_windows[glutGetWindow()]->mouse_click(button, state, x, y);
}

void glut_window::motion_func(int x, int y)
{
	_windows[glutGetWindow()]->mouse_motion(x, y);
}

void glut_window::passive_motion_func(int x, int y)
{
	_windows[glutGetWindow()]->mouse_passive_motion(x, y);
}

void glut_window::wheel_func(int wheel, int direction, int x, int y)
{
	_windows[glutGetWindow()]->mouse_wheel(wheel, direction, x, y);
}

void glut_window::keyboard_func(unsigned char key, int x, int y)
{
	_windows[glutGetWindow()]->key_typed(key, x, y);
}

void glut_window::keyboard_up_func(unsigned char key, int x, int y)
{
	_windows[glutGetWindow()]->key_released(key, x, y);
}

void glut_window::special_func(int key, int x, int y)
{
	_windows[glutGetWindow()]->special_key(key, x, y);
}

void glut_window::special_up_func(int key, int x, int y)
{
	_windows[glutGetWindow()]->special_key_released(key, x, y);
}

}  // gl
