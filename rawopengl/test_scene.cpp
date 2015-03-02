// implementacia normal mapping-u
#include <string>
#include <cassert>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "camera.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "program.hpp"
#include "window.hpp"


class scene_window : public ui::fps_window
{
public:
	using base = fps_window;

	scene_window();
	void display() override;
	void input() override;

	void mouse_passive_motion(int x, int y) override;
	void mouse_click(button b, state s, modifier m, int x, int y) override;

private:
	camera _cam;
	mesh _plane;
	shader::program _prog;
	texture _difftex;
	GLuint _vao;
	bool _freelook = false;
};

void scene_window::input()
{
	float const movement = 0.1f;

	if (key('a'))
		_cam.position -= _cam.right() * movement;

	if (key('d'))
		_cam.position += _cam.right() * movement;

	if (key('w'))
		_cam.position -= _cam.forward() * movement;

	if (key('s'))
		_cam.position += _cam.forward() * movement;

	if (key('z'))
		_cam.position += glm::vec3(0, -1, 0) * movement;

	if (key('x'))
		_cam.position += glm::vec3(0, 1, 0) * movement;

	if (key(27))  // escape
	{
		_freelook = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void scene_window::display()
{
	glm::mat4 M = glm::scale(glm::mat4(1), glm::vec3(5,5,5));
	glm::mat4 V = _cam.view();
	glm::mat4 P = _cam.projection();
	glm::mat4 local_to_screen = P*V*M;

	_prog.use();
	_difftex.bind(0);
	_prog.uniform_variable("tex", 0);
	_prog.uniform_variable("transform", local_to_screen);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_plane.draw();

	base::display();
}

scene_window::scene_window()
	: _cam(glm::vec3(0,1,0), 70, ratio(), 0.01, 1000)
{
	_plane.read("assets/models/plane.obj");
	_prog.read("assets/shaders/view.glsl");
	_difftex.read("assets/textures/bricks.png");

	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);	
}

void scene_window::mouse_passive_motion(int x, int y)
{
	if (!_freelook)
		return;

	unsigned center_w = width()/2;
	unsigned center_h = height()/2;
	float const angular_movement = 0.1f;

	int dx = x - center_w;
	int dy = y - center_h;

	if (dx != 0)
	{
		float angle = angular_movement * dx;
		_cam.rotation = glm::normalize(glm::angleAxis(-angle, glm::vec3(0,1,0)) * _cam.rotation);
	}

	if (dy != 0)
	{
		float angle = angular_movement * dy;
		_cam.rotation = glm::normalize(glm::angleAxis(-angle, _cam.right()) * _cam.rotation);
	}

	if (dx != 0 || dy != 0)
		glutWarpPointer(center_w, center_h);

	base::mouse_passive_motion(x, y);
}

void scene_window::mouse_click(button b, state s, modifier m, int x, int y)
{
	if (b == button::left && s == state::down)
	{
		_freelook = true;
		glutSetCursor(GLUT_CURSOR_NONE);
	}

	base::mouse_click(b, s, m, x, y);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	std::cout << "done!" << std::endl;
	return 0;
}
