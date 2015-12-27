// zobrazenie modelu
#include <functional>
#include <string>
#include <cassert>
#include <sstream>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "camera.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "program.hpp"
#include "window.hpp"
#include "controllers.hpp"
#include "label.hpp"

using std::string;
using gl::camera;
using gl::free_look;
using gl::free_move;
using gl::mesh;
using gl::model;

string const font_path = "/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf";
string const model_path = "assets/bob/bob_lamp.md5mesh";

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

	void camera_position_changed();

private:
	mesh _plane;
	shader::program _prog;
	camera _cam;
	free_look<scene_window> _lookctrl;
	free_move<scene_window> _movectrl;
	model _bob;
	shader::program _normal_vis;
	ui::label<scene_window> _camera_pos_label;
	GLuint _vao;
};

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glm::mat4 M = glm::scale(glm::mat4(1), glm::vec3(5,5,5));
	glm::mat4 V = _cam.view();
	glm::mat4 P = _cam.projection();
	glm::mat4 local_to_screen = P*V*M;
	_prog.use();
	_prog.uniform_variable("color", glm::vec3{0.5, 0.5, 0.5});
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_plane.render();

	M = glm::translate(glm::mat4(1), glm::vec3{0,0,0});
	M = glm::rotate(M, glm::radians(-90.0f), glm::vec3{1,0,0});
	local_to_screen = P*V*M;
	glm::mat3 normal_to_world = glm::mat3{glm::inverseTranspose(M)};
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_world", normal_to_world);
	_bob.render();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_prog.uniform_variable("color", glm::vec3{0,0,0});
	_bob.render();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_normal_vis.use();
	_normal_vis.uniform_variable("local_to_screen", local_to_screen);
	_normal_vis.uniform_variable("normal_length", 0.05f);
	_bob.render();

	_camera_pos_label.render();
	camera_position_changed();

	_movectrl.set_move_callback(std::function<void ()>{std::bind(&scene_window::camera_position_changed, this)});

	base::display();
}

scene_window::scene_window()
	: _lookctrl(_cam, *this), _movectrl(_cam, *this), _camera_pos_label{0, 0, *this}
{
	_cam = camera(glm::vec3(0,6,5), glm::radians(70.0f), aspect_ratio(), 0.01, 1000);
	_plane = gl::mesh_from_file("assets/models/plane.obj");
	_bob = gl::model_from_file(model_path);
	_cam.look_at(glm::vec3{0,4,0});
	_prog.from_file("view.glsl");
	_normal_vis.from_file("normvis.glsl");

	_camera_pos_label.font(font_path, 12);

	std::ostringstream ostr;
	ostr << "camera:(" << _cam.position.x << ", " << _cam.position.y << ", " << _cam.position.z << ")";
	_camera_pos_label.text(ostr.str());

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

void scene_window::camera_position_changed()
{
	std::ostringstream ostr;
	ostr << "camera:(" << _cam.position.x << ", " << _cam.position.y << ", " << _cam.position.z << ")";
	_camera_pos_label.text(ostr.str());
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	std::cout << "done!" << std::endl;
	return 0;
}
