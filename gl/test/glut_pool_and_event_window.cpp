// behaviour based window implementation
#include <iostream>
#include <cassert>
#include <glm/gtx/transform.hpp>
#include "gl/shapes.hpp"
#include "gl/mesh.hpp"
#include "gl/colors.hpp"
#include "gl/program.hpp"
#include "gl/camera.hpp"
#include "gl/default_shaders.hpp"
#include "gl/glut_window.hpp"

using std::cout;
using glm::mat4;
using glm::mat3;
using glm::vec3;
using glm::radians;

class event_scene : public ui::window<ui::event_behaviour, ui::glut_layer>
{
public:
	using base = ui::window<ui::event_behaviour, ui::glut_layer>;

	void display() override
	{
		glClearColor(0,1,1,1);
		glClear(GL_COLOR_BUFFER_BIT);
		base::display();
	}
};


class pool_scene : public ui::window<ui::pool_behaviour, ui::glut_layer>
{
public:
	using base = ui::window<ui::pool_behaviour, ui::glut_layer>;
	pool_scene();
	void display() override;
	void input(float dt) override;

private:
	gl::camera _cam;
	gl::mesh _sphere;
	shader::program _prog;
};

pool_scene::pool_scene()
{
	_cam = gl::camera{vec3{0,0,5}, radians(70.0f), aspect_ratio(), 0.01f, 1000.0f};
	_sphere = gl::make_sphere<gl::mesh>();
	_prog.from_memory(shader::flat_shaded_shader_source);
	glClearColor(1,0,1,1);
}

void pool_scene::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	_prog.use();
	_prog.uniform_variable("local_to_screen", _cam.world_to_screen());
	_prog.uniform_variable("normal_to_world", mat3{1});
	_sphere.render();

	base::display();
}

void pool_scene::input(float dt)
{
	if (in.key_up('a'))
		cout << "keyboard 'a' released" << std::endl;
	if (in.mouse_up(event_handler::button::left))
		cout << "left button released" << std::endl;
	base::input(dt);
}


int main(int argc, char * argv[])
{
	pool_scene w;
	w.start();
	return 0;
}
