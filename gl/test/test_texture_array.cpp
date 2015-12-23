// test pola textur za pouzitia texture2d_array
#include <cassert>
#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/glut_window.hpp"
#include "texture.hpp"
#include "program.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "gl/shapes.hpp"
#include "gl/model_loader.hpp"

using std::unique_ptr;
using std::shared_ptr;
using gl::mesh;
using gl::make_quad_xy;

unique_ptr<uint8_t []> make_pixels(unsigned w);

class main_window : public ui::glut_window
{
public:
	using base = ui::glut_window;

	main_window();
	void display() override;

private:
	shader::program _prog;
	texture2d_array _noise;
	mesh _quad;
};

void main_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	_prog.use();

	_noise.bind(0);
	_prog.uniform_variable("s", 0);

	shared_ptr<shader::uniform> layer_u = _prog.uniform_variable("layer");
	shared_ptr<shader::uniform> transf_u = _prog.uniform_variable("transform");

	// texture 1
	*layer_u = 0;

	glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(-(0.25 + 0.05), 0, 0));
	T = glm::scale(T, glm::vec3(0.25, 0.25, 1));
	*transf_u = T;

	_quad.render();

	// texture 2
	*layer_u = 1;

	T = glm::translate(glm::mat4(1), glm::vec3(0.25 + 0.05, 0, 0));
	T = glm::scale(T, glm::vec3(0.25, 0.25, 1));
	*transf_u = T;

	_quad.render();

	base::display();
}

main_window::main_window() : base(parameters().name("texture-array test"))
{
	unsigned const tile_w = 256;
	unique_ptr<uint8_t []> pixels = make_pixels(tile_w);
	_noise = texture2d_array(tile_w, tile_w, 2, sized_internal_format::rgba8, pixel_format::rgba, pixel_type::ub8, pixels.get());

	_prog.from_file("assets/shaders/texture_array.glsl");
	_quad = make_quad_xy<mesh>();
}

unique_ptr<uint8_t []> make_pixels(unsigned w)
{
	unsigned const ch = 4;
	unsigned const size = w*w*ch;

	unique_ptr<uint8_t []> pixels(new uint8_t[2*size]);

	// texture 1
	for (int i = 0; i < size; i += ch)
	{
		uint8_t * p = pixels.get();
		p[i]   = 255;
		p[i+1] = 0;
		p[i+2] = 0;
		p[i+3] = 255;
	}

	// texture 2
	for (int i = 0; i < size; i += ch)
	{
		uint8_t * p = pixels.get() + size;
		p[i]   = 0;
		p[i+1] = 255;
		p[i+2] = 0;
		p[i+3] = 255;
	}

	return pixels;
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}
