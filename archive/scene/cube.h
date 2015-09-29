#pragma once

#include <GL/glew.h>

namespace gl {

class vbo_cube
{
public:
	vbo_cube();
	void render();

private:
	GLuint _vao;
};

}  // gl
