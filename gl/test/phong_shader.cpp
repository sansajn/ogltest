// phongove osvetlenie
#include <vector>
#include <memory>
#include <string>
#include <glm/gtx/transform.hpp>
#include "gl/colors.hpp"
#include "gl/window.hpp"
#include "gl/shapes.hpp"
#include "gl/scene_object.hpp"
#include "gl/controllers.hpp"
#include "gl/phong.hpp"

using std::string;
using glm::vec3;
using glm::mat4;
using glm::mat3;
using glm::radians;
using gl::mesh;
using gl::free_camera;
using gl::shape_generator;
using ui::glut_pool_window;


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
	shader::phong_light _phong;
	shader::program _prog;
	axis_object _axis;
	light_object _light;
	free_camera<scene_window> _cam;
	shape_generator<mesh> _shape;
};


scene_window::scene_window()
	: base{parameters{}.name("phong shader test")}
	, _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_cube = _shape.cube();
	_box = _shape.box(vec3{.5, 1, 0.5});
	_disk = _shape.disk(.5);
	_cylinder = _shape.cylinder(.5, .5, 30);
	_open_cylinder = _shape.open_cylinder(.5, 1, 20);
	_cone = _shape.cone(.5, 1);
	_sphere = _shape.sphere(.5);
	_prog.from_memory(shader::phong_shader_source);
	_phong.init(&_prog, &_cam.get_camera());
	_cam.get_camera().position = vec3{2,2,3.3};
	_cam.get_camera().look_at(vec3{0,0,0});
}

void scene_window::display()
{
	vec3 light_pos = vec3{10, 20, 30};

	_phong.light(normalize(light_pos), rgb::white, 1.0f);
	_phong.material(vec3{.1f}, 4.0f, .4f);

	shader::program & prog = _phong.shader_program();
	prog.use();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// cube
	mat4 M = mat4{1};
	prog.uniform_variable("color", rgb::lime);
	_phong.render(_cube, M);

	// box
	M = translate(vec3{2, 0, -2});
	prog.uniform_variable("color", rgb::teal);
	_phong.render(_box, M);
	_box.render();

	// disk
	M = translate(vec3{1, 0, 2});
	prog.uniform_variable("color", rgb::yellow);
	_phong.render(_disk, M);

	// cylinder
	M = translate(vec3{-1.5, 0, -.4});
	prog.uniform_variable("color", rgb::olive);
	_phong.render(_cylinder, M);

	// open cylinder
	M = translate(vec3{-.2, 0, -2});
	prog.uniform_variable("color", rgb::maroon);
	_phong.render(_open_cylinder, M);

	// cone
	M = translate(vec3{-2, 0, 1.5});
	prog.uniform_variable("color", rgb::purple);
	_phong.render(_cone, M);

	// sphere
	M = translate(vec3{-.7, 0, 1.8});
	prog.uniform_variable("color", rgb::blue_shades::cornflower_blue);
	_phong.render(_sphere, M);

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
