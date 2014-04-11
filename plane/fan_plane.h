#pragma once
#include <GL/glew.h>

namespace gl {

//! Implementacia roviny pomocou triangle-fan (=> w a h musi byt nasobkom 3).
class vbo_fan_plane
{
public:
	//! \note rozmery w a h musia byt neparne
	vbo_fan_plane(int w, int h);
	~vbo_fan_plane();
	void render();

private:
	int _w, _h;
	GLuint _vao;
};

}  // gl
