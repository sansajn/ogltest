// zobrazi model
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/glut_window.hpp"
#include "gl/model_loader.hpp"
#include "gl/program.hpp"
#include "gl/default_shaders.hpp"
#include "gl/controllers.hpp"
#include "gl/scene_object.hpp"
#include "gl/colors.hpp"

char const * model_path = "../../assets/models/doom3/imp/imp.md5mesh";

using glm::mat4;
using glm::mat3;
using glm::vec3;
using glm::normalize;
using glm::inverseTranspose;
using glm::radians;
using glm::rotate;
using ui::glut_pool_window;
using gl::model;
using gl::model_from_file;
using gl::model_loader_parameters;
using gl::free_camera;
using gl::camera;
using shader::program;

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;
	scene_window();
	void display() override;
	void input(float dt) override;

private:
	model _mdl;
	program _shaded;
	free_camera<scene_window> _view;
	axis_object _axis;
};

scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01f, 1000.0f, *this}
{
	auto params = model_loader_parameters{};
	params.ignore_textures = true;
	_mdl = model_from_file(model_path, params);
	_shaded.from_memory(shader::flat_shaded_shader_source);

	camera & cam = _view.get_camera();
	cam.position = vec3{0, 90, 70};
	cam.look_at(vec3{0, 55, 0});
}

void scene_window::display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera & cam = _view.get_camera();
	mat4 M = rotate(radians(-90.0f), vec3{0,1,0}) * rotate(radians(-90.0f), vec3{1,0,0});
	mat4 world_to_screen = cam.world_to_screen();
	mat4 local_to_screen = world_to_screen * M;
	mat3 normal_to_world = mat3{inverseTranspose(M)};
	vec3 color = vec3{rgb::gray};
	vec3 light_dir = normalize(vec3{1,2,3});

	_shaded.use();
	_shaded.uniform_variable("local_to_screen", local_to_screen);
	_shaded.uniform_variable("normal_to_world", normal_to_world);
	_shaded.uniform_variable("color", color);
	_shaded.uniform_variable("light_dir", light_dir);
	_mdl.render(_shaded);

	_axis.render(world_to_screen);

	base::display();
}

void scene_window::input(float dt)
{
	base::input(dt);
	_view.input(dt);
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
