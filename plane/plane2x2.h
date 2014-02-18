#pragma once

#include <GL/glew.h>


namespace gl {

class vbo_plane2x2
{
public:
	vbo_plane2x2();
	void render();

private:
	GLuint _vao;
};

}  // gl

