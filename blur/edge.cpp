#include <iostream>
#include <vector>
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

mesh make_plane_xy();

class main_window : public ui::glut_window
{
public:
	using base = ui::glut_window;

	main_window();
	~main_window();
	void display() override;
	bool mouse_motion(int x, int y) override;
	bool mouse_click(button b, state s, modifier m, int x, int y) override;
	bool key_typed(unsigned char c, modifier m, int x, int y) override;

private:
	texture _srctex;
	mesh _texframe;
	shader::program _edgeprog;
	shader::program _showprog;
	GLuint _vao;
	float _edge_threshold;
	TwBar * _twbar;
};

main_window::main_window()
	: ui::glut_window(parameters().name("edge detection"))
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	_srctex.open(picture_name);
	_texframe = make_plane_xy();

	_edgeprog.attach(std::shared_ptr<shader::module>(new shader::module("edge.glsl")));
	_showprog.attach(std::shared_ptr<shader::module>(new shader::module("show.glsl")));

	_edge_threshold = 0.5f;

	// ui
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(800, 600);
	_twbar = TwNewBar("options");
	TwAddVarRW(_twbar, "threshold", TW_TYPE_FLOAT, &_edge_threshold, "min=0.0 max=3.0 step=0.01 group='edge detector' help='edge threshold level'");
}

void main_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(_vao);

	// result-image
	float one_over_ratio = 1.0f/ratio();
	float r = _srctex.width() / float(_srctex.height());
	glm::mat4 V(1);  // view transform
	V = glm::scale(V, glm::vec3(one_over_ratio, 1, 1));  // kompenzacia glViewport()
	V = glm::scale(V, glm::vec3(2*r, 2, 1));
	V = glm::translate(V, glm::vec3(-0.5, -0.5, 0));

	_edgeprog.use();
	_srctex.bind(0);
	_edgeprog.uniform_variable("transform", V);
	_edgeprog.uniform_variable("tex", 0);
	_edgeprog.uniform_variable("edge_threshold", _edge_threshold * _edge_threshold);
	_edgeprog.uniform_variable("dx", 1.0f/_srctex.width());
	_edgeprog.uniform_variable("dy", 1.0f/_srctex.height());
	_texframe.draw();

	// source-image
	V = glm::mat4(1);
	V = glm::translate(V, glm::vec3(1-one_over_ratio*r*0.5f, -1, 0));
	V = glm::scale(V, glm::vec3(one_over_ratio*r*0.5f, 0.5f, 1));

	_showprog.use();
	_srctex.bind(0);
	_showprog.uniform_variable("transform", V);
	_showprog.uniform_variable("tex", 0);
	_texframe.draw();

	glBindVertexArray(0);
	TwDraw();

	base::display();
}

main_window::~main_window()
{
	TwDeleteBar(_twbar);
	TwTerminate();
}

bool main_window::mouse_motion(int x, int y)
{
	TwMouseMotion(x, y);
	return base::mouse_motion(x, y);
}

bool main_window::mouse_click(button b, state s, modifier m, int x, int y)
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

	return base::mouse_click(b, s, m, x, y);
}

bool main_window::key_typed(unsigned char c, modifier m, int x, int y)
{
	TwKeyPressed(c, 0);  // TODO: modifiers not handled
	return base::key_typed(c, m, x, y);
}

mesh make_plane_xy()
{
	std::vector<vertex> verts{
		{glm::vec3(0,0,0), glm::vec2(0,0)},
		{glm::vec3(1,0,0), glm::vec2(1,0)},
		{glm::vec3(1,1,0), glm::vec2(1,1)},
		{glm::vec3(0,1,0), glm::vec2(0,1)}
	};

	std::vector<unsigned> indices{0,1,2, 2,3,0};

	return mesh(verts, indices);
}


int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}
