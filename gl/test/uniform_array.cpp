// test uniform array
#include <utility>
#include <string>
#include <vector>
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
#include "window.hpp"
#include "controllers.hpp"
#include "gl/shapes.hpp"
#include "gl/model_loader.hpp"

// TODO: osvetlenie je nejake divne (pozri tie paprsky na hranach trojuholnika)

using std::make_pair;
using std::string;
using std::vector;
using gl::mesh;
using gl::make_sphere;
using gl::make_cube;
using gl::camera;
using gl::free_look;
using gl::free_move;

string monkey_path = "assets/models/monkey.obj";
string plane_path = "assets/models/plane.obj";

string const shader_source{
	"uniform mat4 T[3];  // M, V and P transformation\n\
	#ifdef _VERTEX_\n\
	layout(location = 0) in vec3 position;\n\
	layout(location = 2) in vec3 normal;\n\
	out vec3 n;  // normal in local space\n\
	void main() {\n\
		n = normal;\n\
		gl_Position = T[2] * T[1] * T[0] * vec4(position, 1);\n\
	}\n\
	#endif  // _VERTEX_\n\
	#ifdef _FRAGMENT_\n\
	uniform vec3 color = vec3(0.7, 0.7, 0.7);\n\
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
	mesh _monkey;
	mesh _cube;
	mesh _sphere;
	mesh _plane;
	glm::vec3 _monkey_pos;
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

	glm::mat4 T[3];
	T[1] = V;
	T[2] = P;

	_prog.use();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glm::mat4 M = glm::translate(glm::mat4(1), _monkey_pos);
	T[0] = M;
	vector<glm::mat4> MVP{T, T+3};
	_prog.uniform_variable("T", MVP);  // vector version test
	_prog.uniform_variable("color", glm::vec3{0.7, 0, 0});
	_monkey.render();

	M = glm::translate(glm::mat4{1}, glm::vec3{-3, 0, 0} + _monkey_pos);
	T[0] = M;
	_prog.uniform_variable("T", make_pair(T, 3));  // pair test
	_prog.uniform_variable("color", glm::vec3{0, 0.65, 0});
	_cube.render();

	M = glm::translate(glm::mat4{1}, glm::vec3{-1,0,-3} + _monkey_pos);
	T[0] = M;
	_prog.uniform_variable("T", make_pair(T, 3));
	_prog.uniform_variable("color", glm::vec3{0, 0, 0.4});
	_sphere.render();

	M = glm::scale(glm::mat4{1}, glm::vec3{10,10,10});
	T[0] = M;
	_prog.uniform_variable("T", make_pair(T, 3));
	_prog.uniform_variable("color", glm::vec3{0.7, 0.7, 0.7});
	_plane.render();

	base::display();
}

scene_window::scene_window()
	: _lookctrl(_cam, *this), _movectrl(_cam, *this)
{
	_cam = camera(glm::vec3(0,2,0), glm::radians(70.0f), aspect_ratio(), 0.01, 1000);
	_prog.from_memory(shader_source);
	_monkey = gl::mesh_from_file(monkey_path);
	_monkey_pos = glm::vec3(3,1,-3);
	_cube = make_cube<mesh>();
	_sphere = make_sphere<mesh>();
	_plane = gl::mesh_from_file(plane_path);

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
