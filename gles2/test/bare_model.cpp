// precita model pomocou gles2 kompatibilneho shaderu (trieda model)
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/glfw3_window.hpp"
#include "gl/free_camera.hpp"
#include "gl/colors.hpp"
#include "gles2/model_loader_gles2.hpp"
#include "gles2/light_gles2.hpp"
#include "gles2/default_shader_gles2.hpp"

char const * model_path = "../../assets/models/bob_lamp/bob_lamp.md5mesh";

using std::string;
using std::shared_ptr;
using std::vector;
using std::runtime_error;
using glm::vec3;
using glm::vec2;
using glm::mat4;
using glm::mat3;
using glm::radians;
using glm::rotate;
using glm::inverseTranspose;
using gl::camera;
using gl::free_camera;
using gles2::model;
using gles2::model_from_file;
using gles2::model_loader_parameters;
using gles2::material_property;
using gles2::diffuse_light;
using gles2::shader::program;


class scene_window : public ui::glfw_pool_window
{
public:
	using base = ui::glfw_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	program _prog;
	model _mdl;
	free_camera<scene_window> _view;
	diffuse_light _light;
};

scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01f, 1000.0f, *this}
{
	_prog.from_memory(gles2::flat_shaded_shader_source);

	auto model_params = model_loader_parameters{};
	model_params.ignore_textures = true;
	_mdl = model_from_file(model_path, model_params);

	// toto treba robyt podla toho, ako je vytvoreny model
	int position_a = _prog.attribute_location("position");
	int texcoord_a = _prog.attribute_location("texcoord");
	int normal_a = _prog.attribute_location("normal");
	int tangent_a = _prog.attribute_location("tangent");
	_mdl.attribute_location({position_a, texcoord_a, normal_a, tangent_a});

	_view.get_camera().position = vec3{0, 6, 6};
	_view.get_camera().look_at(vec3{0, 3, 0});

	_light = diffuse_light{normalize(vec3{2,3,5})};
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera & cam = _view.get_camera();
	mat4 M = rotate(radians(-90.0f), vec3{1,0,0});
	mat4 world_to_camera = cam.view();
	mat4 local_to_camera = world_to_camera * M;
	mat4 local_to_screen = cam.projection() * local_to_camera;
	mat3 normal_to_world = mat3{inverseTranspose(M)};
	_prog.use();
	_light.apply(_prog);
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_world", normal_to_world);
	_prog.uniform_variable("color", rgb::olive);

	glEnable(GL_DEPTH_TEST);
	_mdl.render(_prog);

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
