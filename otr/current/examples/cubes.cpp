/* nakresli tri kocky */
#include <memory>
#include <iostream>
#include <cassert>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "render/program.h"
#include "render/buffer.h"
#include "render/framebuffer.h"
#include "ui/sdl_window.h"
//#include "render/mesh.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))

int const WIDTH = 800;
int const HEIGHT = 600;


struct P3_N3_UV_C
{
	float x, y, z, nx, ny, nz, u, v;
	float r, g, b, a;

	P3_N3_UV_C() {}
	P3_N3_UV_C(float x, float y, float z, float nx, float ny, float nz,
		float u, float v, float r, float g, float b, float a) :
		  x(x), y(y), z(z), nx(nx), ny(ny), nz(nz), u(u), v(v), r(r), g(g), b(b), a(a)
	{}
};


class app_window
	: public sdl_window
{
public:
	typedef sdl_window base;

	app_window();
	void display();
	void reshape(int w, int h);

private:
	gpubuffer _buf;
	shader_program _prog;
	GLuint _vao;
	std::shared_ptr<mesh<P3_N3_UV_C, unsigned int>> _cube;
	glm::mat4 _VP;  // view-perspective projection
	ptr<uniform_variable> _u_mvp;
};

app_window::app_window()
	: base(window::parameters().size(WIDTH, HEIGHT).version(3, 3).name("cubes"))
{
	_cube = std::make_shared<mesh<P3_N3_UV_C, unsigned int>>(mesh_mode::triangles, mesh_usage::GPU_STATIC);
	_cube->append_attribute_type(0, 3, attribute_type::f32, false);
	_cube->append_attribute_type(1, 3, attribute_type::f32, false);
	_cube->append_attribute_type(2, 2, attribute_type::f32, false);
	_cube->append_attribute_type(3, 4, attribute_type::f32, false);
	_cube->append_vertex(P3_N3_UV_C(-1, -1, +1, 0, 0, +1, 0, 0, 1, 0, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, +1, 0, 0, +1, 1, 0, 1, 0, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, +1, 0, 0, +1, 1, 1, 1, 0, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, +1, 0, 0, +1, 1, 1, 1, 0, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, +1, 0, 0, +1, 0, 1, 1, 0, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, -1, +1, 0, 0, +1, 0, 0, 1, 0, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, +1, +1, 0, 0, 0, 0, 0, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, -1, +1, 0, 0, 1, 0, 0, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, -1, +1, 0, 0, 1, 1, 0, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, -1, +1, 0, 0, 1, 1, 0, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, +1, +1, 0, 0, 0, 1, 0, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, +1, +1, 0, 0, 0, 0, 0, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, +1, 0, +1, 0, 0, 0, 0, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, +1, 0, +1, 0, 1, 0, 0, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, -1, 0, +1, 0, 1, 1, 0, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, -1, 0, +1, 0, 1, 1, 0, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, -1, 0, +1, 0, 0, 1, 0, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, +1, 0, +1, 0, 0, 0, 0, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, -1, 0, 0, -1, 0, 0, 0, 1, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, -1, -1, 0, 0, -1, 1, 0, 0, 1, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, -1, 0, 0, -1, 1, 1, 0, 1, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, -1, 0, 0, -1, 1, 1, 0, 1, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, +1, -1, 0, 0, -1, 0, 1, 0, 1, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, -1, 0, 0, -1, 0, 0, 0, 1, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, -1, -1, -1, 0, 0, 0, 0, 1, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, -1, +1, -1, 0, 0, 1, 0, 1, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, +1, -1, 0, 0, 1, 1, 1, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, +1, -1, 0, 0, 1, 1, 1, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, +1, -1, -1, 0, 0, 0, 1, 1, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, -1, -1, -1, 0, 0, 0, 0, 1, 0, 1, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, -1, -1, 0, -1, 0, 0, 0, 1, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, -1, 0, -1, 0, 1, 0, 1, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, +1, 0, -1, 0, 1, 1, 1, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(+1, -1, +1, 0, -1, 0, 1, 1, 1, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, -1, +1, 0, -1, 0, 0, 1, 1, 1, 0, 1));
	_cube->append_vertex(P3_N3_UV_C(-1, -1, -1, 0, -1, 0, 0, 0, 1, 1, 0, 1));

	_prog << "shader/cubes.vs" << "shader/cubes.fs";
	_prog.link();
	_prog.use();

	_u_mvp = make_ptr<uniform_variable>("MVP", _prog);

	glm::mat4 P = glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(5.0f, 5.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	_VP = P*V;

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	framebuffer & fb = framebuffer::default_fb();
	fb.depth_test(true);
}

void app_window::display()
{
	framebuffer & fb = framebuffer::default_fb();
	fb.clear(true, true);

	glm::mat4 M(1.0f);
	glm::mat4 MVP = _VP*M;
	*_u_mvp = MVP;
	fb.draw(_prog, *_cube);

	M = glm::translate(glm::mat4(1.0f), glm::vec3(2.5f, 0.0f, 1.0f));
	MVP = _VP*M;
	*_u_mvp = MVP;
	fb.draw(_prog, *_cube);

	M = glm::translate(glm::mat4(1.0f), glm::vec3(2.25f, 0.0f, -1.5f));
	MVP = _VP*M;
	*_u_mvp = MVP;
	fb.draw(_prog, *_cube);

	base::display();
}

void app_window::reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	base::reshape(w, h);
}


int main(int argc, char * argv[])
{
	app_window w;
	w.start();
	return 0;
}
