#include <cassert>
#include <GL/glew.h>
#include "window.h"

namespace gl {

void window::glew_init()
{
	assert(glGetError() == 0);
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "GLEW error: inicialization failed");  // TODO: throw exception
	glGetError();  // eat error
}

window::window()
{
//	glew_init();
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
