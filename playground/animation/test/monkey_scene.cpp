// scena s rovinou opicou a texturou
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
#include "controllers.hpp"

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	mesh _plane;
	shader::program _prog;
	texture2d _difftex;
	camera _cam;
	free_look<scene_window> _lookctrl;
	free_move<scene_window> _movectrl;
	mesh _monkey;
	glm::vec3 _monkey_pos;
	GLuint _vao;
};

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

	M = glm::translate(glm::mat4(1), _monkey_pos);
	local_to_screen = P*V*M;
	_prog.uniform_variable("transform", local_to_screen);
	_monkey.draw();

	base::display();
}

scene_window::scene_window()
	: _lookctrl(_cam, *this), _movectrl(_cam, *this)
{
	_cam = camera(glm::vec3(0,1,0), glm::radians(70.0f), aspect_ratio(), 0.01, 1000);
	_plane = mesh_from_file("../assets/models/plane.obj");
	_prog.from_file("../assets/shaders/view.glsl");
	_difftex = texture2d("../assets/textures/bricks.png");
	_monkey = mesh_from_file("../assets/models/monkey.obj");
	_monkey_pos = glm::vec3(3,1,-3);

	_cam.look_at(_monkey_pos);

	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);	
}

void scene_window::input(float dt)
{
	_lookctrl.input(dt);
	_movectrl.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	std::cout << "done!" << std::endl;
	return 0;
}
