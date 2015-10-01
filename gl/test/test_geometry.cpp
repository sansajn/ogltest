/* test geometry shader programu (vyzualizuje normaly modelu) */
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "mesh.hpp"
#include "camera.hpp"
#include "program.hpp"
#include "window.hpp"
#include "controllers.hpp"

using gl::mesh;
using gl::make_plane_xz;
using gl::camera;
using gl::free_look;
using gl::free_move;

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	mesh _plane;
	camera _cam;
	shader::program _show;
	shader::program _shownorm;
	free_look<scene_window> _lookctrl;
	free_move<scene_window> _movectrl;
	GLuint _vao;
};

scene_window::scene_window()
	: _lookctrl(_cam, *this), _movectrl(_cam, *this)
{
	_plane = make_plane_xz(10, 10);
	_cam = camera(glm::vec3(0,1,0), glm::radians(70.0f), aspect_ratio(), 0.01, 1000.0);
	_show.from_file("assets/shaders/geometry_view.glsl");
	_shownorm.from_file("assets/shaders/geometry_norm.glsl");

	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_shownorm.use();
	glm::mat4 local_to_world = glm::scale(glm::mat4(1), glm::vec3(5, 5, 5));
	glm::mat4 local_to_screen = _cam.view_projection() * local_to_world;
	_shownorm.uniform_variable("local_to_screen", local_to_screen);
	_shownorm.uniform_variable("normal_length", 0.01f);
	_plane.render();

	_show.use();
	_show.uniform_variable("local_to_screen", local_to_screen);
	_plane.render();

	base::display();
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
	return 0;
}
