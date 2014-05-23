#include <cassert>
#include <GL/glew.h>
#include "window.h"

namespace gl {

static void glew_init()
{
	assert(glGetError() == 0);
	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();  // eat error
}

window::window()
{
	glew_init();
}

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

};  // gl
