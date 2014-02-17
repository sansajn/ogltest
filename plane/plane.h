#pragma once

#include <GL/glew.h>

namespace gl {

class vbo_plane
{
public:
	vbo_plane(int w, int h);
	void render();

private:
	int _w, _h;
	GLuint _vao;
};

}  // gl
