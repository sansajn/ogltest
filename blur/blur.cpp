#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <cmath>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <ImageMagick/Magick++.h>
#include "window.hpp"
#include "texture.hpp"
#include "program.hpp"
#include "mesh.hpp"

char const * picture_name = "lena.png";

using std::cout;
using std::string;
using std::make_pair;

mesh make_plane_xy();
float normpdf(float x, float sigma);

class bitmap
{
public:
	bitmap() {}
	void open(std::string const & file_name) {_im.read(file_name);}

	int width() const {return int(_im.columns());}
	int height() const {return int(_im.rows());}

private:
	Magick::Image _im;
};

class main_window : public ui::glut_window
{
public:
	using base = ui::glut_window;

	main_window();
	void display();

private:
	texture _srctex;
	texture _fbtex1;
	texture _fbtex2;
	mesh _texframe;
	shader::program _viewprog;
	shader::program _blurprog;
	GLuint _vao;
	float _weights[5];  // normpdf weights
};

main_window::main_window()
	: ui::glut_window(parameters().name("blur example").version(3,3))
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	_srctex.open(picture_name);
	int w = _srctex.width(), h = _srctex.height();
	_fbtex1.create(w, h);
	_fbtex2.create(w, h);
	_texframe = make_plane_xy();

	std::shared_ptr<shader::module> blur_module(new shader::module("blur.glsl"));
	_blurprog.attach(blur_module);

	std::shared_ptr<shader::module> view_module(new shader::module("show.glsl"));
	_viewprog.attach(view_module);

	// compute filter weights
	float const sigma = 2;
	_weights[0] = normpdf(0, sigma);
	float sum = _weights[0];
	for (int i = 1; i < 5; ++i)
	{
		_weights[i] = normpdf(i, sigma);
		sum += 2*_weights[i];
	}

	for (int i = 0; i < 5; ++i)
		_weights[i] /= sum;
}

void main_window::display()
{
	// filter phase
	_blurprog.use();
	_fbtex1.bind_as_render_target();
	_srctex.bind(0);
	_blurprog.uniform_variable("tex", 0);

	// weights
	_blurprog.uniform_variable("weights[0]", make_pair(_weights, 5));

	// x direction filtering
	_blurprog.uniform_variable("direction", glm::vec2(1.0f/_srctex.width(), 0.0f));
	_texframe.draw();

	// y direction filtering
	_fbtex2.bind_as_render_target();
	_fbtex1.bind(0);
	_blurprog.uniform_variable("direction", glm::vec2(1.0f/_srctex.height(), 0.0f));

	_texframe.draw();

	texture & resulttex = _fbtex2;

	// show phase
	glm::mat4 V(1);
	V = glm::scale(V, glm::vec3(1/ratio(), 1, 1));
	_viewprog.use();
	bind_as_render_target();
	resulttex.bind(0);
	_viewprog.uniform_variable("tex", 0);
	_viewprog.uniform_variable("transform", V);
	_texframe.draw();

	base::display();
}

mesh make_plane_xy()
{
	std::vector<vertex> verts{
		{glm::vec3(-1,-1,0), glm::vec2(0,0)},
		{glm::vec3(1,-1,0), glm::vec2(1,0)},
		{glm::vec3(1,1,0), glm::vec2(1,1)},
		{glm::vec3(-1,1,0), glm::vec2(0,1)}
	};

	std::vector<unsigned> indices{0,1,2, 2,3,0};

	return mesh(verts, indices);
}

float normpdf(float x, float sigma)
{
	float k = x/sigma;
	return exp(-0.5f*k*k) / sqrt(2*M_PI)*sigma;
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}
