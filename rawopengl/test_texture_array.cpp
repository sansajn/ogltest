#include <memory>
#include <utility>
#include <GL/glew.h>
#include "window.hpp"
#include "program.hpp"
#include "mesh.hpp"
#include "texture.hpp"

using std::unique_ptr;

unique_ptr<uint8_t []> make_pixels(unsigned w);

class scene_window : public ui::glut_window
{
public:
	using base = ui::glut_window;

	scene_window();
	void display() override;

private:
	GLuint _tid;
	shader::program _prog;
	mesh _quad;
	GLuint _vao;
	texture_array _tarray;
};

scene_window::scene_window()
{
	unsigned const tile_w = 4;
	unsigned const layers = 2;

	unique_ptr<uint8_t []> pixels = make_pixels(tile_w);

	_tarray = texture_array(tile_w, tile_w, layers, pixels.get());

	_prog.read("assets/shaders/texture_array.glsl");
	_quad = make_plane_xy();

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	_prog.use();

	// bind
	_tarray.bind(0);

	_prog.uniform_variable("s", 0);
	_prog.uniform_variable("texture_idx", 1);
	_quad.draw();

	base::display();
}

unique_ptr<uint8_t []> make_pixels(unsigned w)
{
	int const ch = 4;
	int const npixel_values = w*w*ch;

	unique_ptr<uint8_t []> pixels(new uint8_t[2*npixel_values]);

	// texture 1
	for (int i = 0; i < npixel_values; i += ch)
	{
		uint8_t * p = pixels.get();
		p[i]   = 255;
		p[i+1] = 0;
		p[i+2] = 0;
		p[i+3] = 255;
	}

	// texture 2
	for (int i = 0; i < npixel_values; i += ch)
	{
		uint8_t * p = pixels.get() + npixel_values;
		p[i]   = 0;
		p[i+1] = 255;
		p[i+2] = 0;
		p[i+3] = 255;
	}

	return pixels;
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
