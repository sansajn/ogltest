// gl::free_look sample
#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/matrix_inverse.hpp>
#include "gles2/program_gles2.hpp"
#include "gles2/mesh_gles2.hpp"
#include "gles2/default_shader_gles2.hpp"
#include "gl/camera.hpp"
#include "gl/colors.hpp"
#include "gl/shapes.hpp"
#include "gl/glfw3_window.hpp"
#include "gl/free_look.hpp"

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;

using glm::vec3;
using glm::mat4;
using glm::mat3;
using glm::radians;
using glm::inverseTranspose;
using gl::camera;
using gl::free_look;
using gles2::shader::program;
using gles2::mesh;


class scene : public ui::glfw_window
{
public:
	using base = ui::glfw_window;

	scene();
	void display() override;
	void input(float dt) override;

private:
	camera _cam;
	free_look<scene> _look;
	program _shaded;
	mesh _cube;
	vec3 _light_pos;
};

void scene::display()
{
	GLint position_a = glGetAttribLocation(_shaded.id(), "position");
	GLint normal_a = glGetAttribLocation(_shaded.id(), "normal");

	mat4 VP = _cam.view_projection();
	mat4 M = mat4{1};
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::gray);
	_shaded.uniform_variable("light_dir", normalize(_light_pos));

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	_cube.attribute_location({position_a, normal_a});
	_cube.render();

	base::display();
}

void scene::input(float dt)
{
	base::input(dt);

	_look.input(dt);
}

scene::scene()
	: _cam{radians(70.0f), WIDTH/(float)HEIGHT, 0.01f, 1000.0f}
	, _look{_cam, *this}
	, _light_pos{10, 20, 30}
{
	_cam.position = vec3{2,2,5};
	_cam.look_at(vec3{0,0,0});

	_shaded.from_memory(gles2::flat_shaded_shader_source, 100);
	GLint pos = _shaded.attribute_location("position");
	glEnableVertexAttribArray(pos);

	_shaded.use();

	gl::shape_generator<gles2::mesh> shape;
	_cube = shape.cube();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, WIDTH, HEIGHT);
}

int main(int argc, char * argv[])
{
	scene s;
	s.start();
	return 0;
}
