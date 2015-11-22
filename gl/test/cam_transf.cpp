// model s relativnou polohou voci kamere
#include <string>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/glew.h>
#include "window.hpp"
#include "program.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "colors.hpp"
#include "scene_object.hpp"
#include "shapes.hpp"

using std::string;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::mat4_cast;
using glm::normalize;
using glm::translate;
using glm::radians;
using glm::inverseTranspose;
using glm::conjugate;
using glm::inverse;
using gl::mesh;
using gl::camera;
using gl::free_look;
using gl::make_cube;
using gl::make_sphere;
using ui::glut_pool_window;

string cube_shader_path = "assets/shaders/shaded.glsl";

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	mesh _cube;
	camera _cam;
	free_look<scene_window> _look;
	shader::program _cube_prog;
	axis_object _axis;
	light_object _light;
};

scene_window::scene_window() : _look(_cam, *this), _light{rgb::yellow}
{
	_cube = make_cube();
	_cube_prog.from_file(cube_shader_path);

	_cam = camera{radians(70.0f), aspect_ratio(), 0.01, 1000};
//	_cam.position = vec3{3,3,3};
	_cam.position = vec3{0,0,3};

	glClearColor(0, 0, 0, 1);
}

void scene_window::display()
{
	vec3 const light_pos{10,10,10};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// cube
	mat4 M = inverse(_cam.view()) * translate(-_cam.position);
	mat4 local_to_screen = _cam.view_projection() * M;
	mat3 normal_to_world = mat3{inverseTranspose(M)};
	_cube_prog.use();
	_cube_prog.uniform_variable("local_to_screen", local_to_screen);
	_cube_prog.uniform_variable("normal_to_world", normal_to_world);
	_cube_prog.uniform_variable("light_dir", normalize(light_pos));
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_cube.render();

	_axis.render(_cam.view_projection());
	_light.render(_cam.view_projection() * translate(light_pos));

	base::display();
}

void scene_window::input(float dt)
{
	_look.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
