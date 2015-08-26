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
#include <AntTweakBar.h>
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

class main_window : public ui::glut_window
{
public:
	using base = ui::glut_window;

	main_window();
	~main_window();
	void display() override;
	void mouse_motion(int x, int y) override;
	void mouse_click(button b, state s, modifier m, int x, int y) override;
	void key_typed(unsigned char c, modifier m, int x, int y) override;

private:
	void compute_filter_weights();

	texture _srctex;
	texture _fbtex1;
	texture _fbtex2;
	mesh _texframe;
	shader::program _viewprog;
	shader::program _blurprog;
	GLuint _vao;
	float _weights[5];  // normpdf weights
	TwBar * _twbar;
	float _sigma;
};

main_window::main_window()
	: ui::glut_window(parameters().name("blur example"))
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

	_sigma = 2.0f;

	// ui
	TwInit(TW_OPENGL, nullptr);
	TwWindowSize(width(), height());
	_twbar = TwNewBar("options");
	TwAddVarRW(_twbar, "sigma", TW_TYPE_FLOAT, &_sigma, "min=0.1 max=10.0 step=0.05 group='blur' help='blur amoungth'");
}

void main_window::display()
{
	// filter phase
	_blurprog.use();
	_fbtex1.bind_as_render_target();
	_srctex.bind(0);
	_blurprog.uniform_variable("tex", 0);

	// weights
	compute_filter_weights();
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

	// ui
	glBindVertexArray(0);
	TwDraw();

	base::display();
}

void main_window::compute_filter_weights()
{
	_weights[0] = normpdf(0, _sigma);
	float sum = _weights[0];
	for (int i = 1; i < 5; ++i)
	{
		_weights[i] = normpdf(i, _sigma);
		sum += 2*_weights[i];
	}

	for (int i = 0; i < 5; ++i)
		_weights[i] /= sum;
}

main_window::~main_window()
{
	TwDeleteBar(_twbar);
	TwTerminate();
}

void main_window::mouse_motion(int x, int y)
{
	TwMouseMotion(x, y);
	base::mouse_motion(x, y);
}

void main_window::mouse_click(button b, state s, modifier m, int x, int y)
{
	TwMouseMotion(x, y);

	TwMouseAction action = (s == state::down) ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
	switch (b)
	{
		case button::left:
			TwMouseButton(action, TW_MOUSE_LEFT);
			break;

		case button::middle:
			TwMouseButton(action, TW_MOUSE_MIDDLE);
			break;

		case button::right:
			TwMouseButton(action, TW_MOUSE_RIGHT);
			break;

		default:
			// TODO: support wheel events
			break;
	}

	base::mouse_click(b, s, m, x, y);
}

void main_window::key_typed(unsigned char c, modifier m, int x, int y)
{
	TwKeyPressed(c, 0);  // TODO: modifiers not handled
	base::key_typed(c, m, x, y);
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
