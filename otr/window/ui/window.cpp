#include "window.hpp"
#include <cassert>
#include <GL/glew.h>


window::parameters::parameters()
{
	_size = std::make_pair(800, 600);
	_name = "OpenGL Window";
	_version = std::make_pair(-1, -1);
	_debug = false;
}

window::parameters & window::parameters::version(int major, int minor)
{
	_version = std::make_pair(major, minor);
	return *this;
}

void window::glew_init()
{
	assert(glGetError() == 0);
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
		throw window_error("GLEW inicialization failed");
	glGetError();  // eat error
}

window::window()
{
}

void window::reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}
