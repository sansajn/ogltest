// kocka
#include <vector>
#include <string>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "window.hpp"
#include "controllers.hpp"
#include "scene_object.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "shapes.hpp"
#include "colors.hpp"

using std::string;
using std::vector;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::normalize;
using glm::radians;
using glm::inverseTranspose;
using gl::mesh;
using gl::attribute;
using gl::free_camera;
using ui::glut_pool_window;

string shaded_shader_path = "assets/shaders/shaded.glsl";

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;
	scene_window();
	void input(float dt) override;
	void display() override;

private:
	mesh _cube;
	mesh _box;
	mesh _disk;
	mesh _cylinder;
	mesh _open_cylinder;
	mesh _cone;
	mesh _sphere;
	shader::program _prog;
	axis_object _axis;
	light_object _light;
	free_camera<scene_window> _cam;
};


scene_window::scene_window() : _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_cube = gl::make_cube();
	_box = gl::make_box(vec3{.5, 1, 0.5});
	_disk = gl::make_disk(.5);
	_cylinder = gl::make_cylinder(.5, .5, 10);
	_open_cylinder = gl::make_open_cylinder(.5, 1, 20);
	_cone = gl::make_cone(.5, 1);
	_sphere = gl::make_sphere(.5);
	_prog.from_file(shaded_shader_path);
	_cam.get_camera().position = vec3{2,2,3.3};
	_cam.get_camera().look_at(vec3{0,0,0});
}

void scene_window::display()
{
	vec3 light_pos = vec3{10, 20, 30};

	_prog.use();
	// cube
	mat4 VP = _cam.get_camera().view_projection();
	mat4 M = mat4{1};
	_prog.uniform_variable("local_to_screen", VP*M);
	_prog.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_prog.uniform_variable("color", rgb::gray);
	_prog.uniform_variable("light_dir", normalize(light_pos));

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	_cube.render();

	// box
	M = translate(vec3{2, 0, -2});
	_prog.uniform_variable("local_to_screen", VP*M);
	_prog.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_prog.uniform_variable("color", rgb::teal);
	_box.render();

	// disk
	M = translate(vec3{1, 0, 2});
	_prog.uniform_variable("local_to_screen", VP*M);
	_prog.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_prog.uniform_variable("color", rgb::yellow);
	_disk.render();

	// cylinder
	M = translate(vec3{-1.5, 0, -.4});
	_prog.uniform_variable("local_to_screen", VP*M);
	_prog.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_prog.uniform_variable("color", rgb::olive);
	_cylinder.render();

	// open cylinder
	M = translate(vec3{-.2, 0, -2});
	_prog.uniform_variable("local_to_screen", VP*M);
	_prog.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_prog.uniform_variable("color", rgb::maroon);
	_open_cylinder.render();

	// cone
	M = translate(vec3{-2, 0, 1.5});
	_prog.uniform_variable("local_to_screen", VP*M);
	_prog.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_prog.uniform_variable("color", rgb::purple);
	_cone.render();

	// sphere
	M = translate(vec3{-.7, 0, 1.8});
	_prog.uniform_variable("local_to_screen", VP*M);
	_prog.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_prog.uniform_variable("color", rgb::blue_shades::cornflower_blue);
	_sphere.render();

	_axis.render(_cam.get_camera().view_projection());
	_light.render(_cam.get_camera().view_projection() * translate(light_pos));

	base::display();
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
	return 0;
}

