// test mesh implementacie
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
#include "program.hpp"
#include "gl/glut_window.hpp"
#include "controllers.hpp"
#include "gl/shapes.hpp"

// TODO: osvetlenie je nejake divne (pozri tie paprsky na hranach trojuholnika)

using std::string;
using gl::mesh;
using gl::make_plane_xy;
using gl::camera;
using gl::free_look;
using gl::free_move;

string const shader_source{
	"uniform mat4 local_to_screen;\n\
	uniform vec3 color = vec3(0.7, 0.7, 0.7);\n\
	#ifdef _VERTEX_\n\
	layout(location = 0) in vec3 position;\n\
	layout(location = 2) in vec3 normal;\n\
	out vec3 n;  // normal in local space\n\
	void main() {\n\
		n = normal;\n\
		gl_Position = local_to_screen * vec4(position, 1);\n\
	}\n\
	#endif  // _VERTEX_\n\
	#ifdef _FRAGMENT_\n\
	in vec3 n;  // in local space\n\
	out vec4 fcolor;\n\
	const vec3 light_direction = normalize(vec3(1));\n\
	void main() {\n\
		float light_intensity = max(dot(normalize(n), light_direction), 0);\n\
		fcolor = vec4(min(0.3 + light_intensity, 1) * color, 1);\n\
	}\n\
	#endif  // _FRAGMENT_"};

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
	camera _cam;
	free_look<scene_window> _lookctrl;
	free_move<scene_window> _movectrl;
	GLuint _vao;
};

void scene_window::display()
{
	glm::mat4 V = _cam.view();
	glm::mat4 P = _cam.projection();

	_prog.use();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glm::mat4 M{1};
	glm::mat4 local_to_screen = P*V*M;
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_plane.render();

	base::display();
}

scene_window::scene_window()
	: _lookctrl(_cam, *this), _movectrl(_cam, *this)
{
	double size = 1.0;
	_cam = camera(glm::vec3(0, 0, 5), glm::radians(70.0f), aspect_ratio(), 0.01, 1000);
	_prog.from_memory(shader_source);
	_plane = make_plane_xy<mesh>(glm::vec3{-size, -size, 0}, 2*size, 11, 11);

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
