// generovanie mipmap textury
#include <string>
#include <cassert>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "gl/mesh.hpp"
#include "gl/program.hpp"
#include "gl/window.hpp"
#include "gl/shapes.hpp"
#include "gl/controllers.hpp"
#include "gl/texture_loader.hpp"

char const * texture_path = "assets/textures/bricks.png";
char const * shader_program_path = "assets/shaders/view.glsl";

using glm::vec3;
using glm::radians;
using gl::mesh;
using gl::free_camera;
using gl::make_plane_xz;
using gl::texture_from_file;

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
	free_camera<scene_window> _cam;
};

void scene_window::display()
{
	glm::mat4 M{1};// = glm::scale(glm::mat4(1), glm::vec3(5,5,5));
	glm::mat4 local_to_screen = _cam.get_camera().world_to_screen() * M;

	_prog.use();
	_difftex.bind(0);
	_prog.uniform_variable("tex", 0);
	_prog.uniform_variable("transform", local_to_screen);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_plane.render();

	base::display();
}

scene_window::scene_window() : _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_plane = make_plane_xz<mesh>(20, 20, 5.0);
	_prog.from_file(shader_program_path);
	_difftex = texture_from_file(texture_path,
		texture::parameters{}.min(texture_filter::linear_mipmap_linear).wrap(texture_wrap::repeat));
	_cam.get_camera().position = vec3{2.5, 0.25,-2.5};
	_cam.get_camera().look_at(vec3{0,0,0});
	glEnable(GL_DEPTH_TEST);
}

void scene_window::input(float dt)
{
	_cam.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	std::cout << "done!" << std::endl;
	return 0;
}

