// texturovany model
#include <memory>
#include <string>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/glut_window.hpp"
#include "gl/program.hpp"
#include "gl/controllers.hpp"
#include "gl/texture.hpp"
#include "gl/shapes.hpp"
#include "gl/model_loader.hpp"

using std::string;
using glm::mat4;
using glm::mat3;
using glm::vec3;
using glm::translate;
using glm::scale;
using glm::normalize;
using glm::inverseTranspose;
using glm::radians;
using glm::rotate;
using glm::mat3_cast;
using gl::mesh;
using gl::attribute;
using gl::model;
using gl::camera;
using gl::free_look;
using gl::free_move;
using ui::glut_pool_window;

char const * model_path = "assets/blaster/view.md5mesh";
char const * shaded_shader_path = "shaders/bumped.glsl";
char const * axis_shader_path = "shaders/colored.glsl";
char const * light_shader_path = "shaders/solid.glsl";

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	model _textured;
	shader::program _prog;
	camera _cam;
	free_look<scene_window> _look;
	free_move<scene_window> _move;

	// debug stuff
	mesh _axis;
	mesh _light;
	shader::program _axis_prog;
	shader::program _light_prog;
};

scene_window::scene_window()
	: base{parameters{}.name("textured_model")}, _look{_cam, *this}, _move{_cam, *this}
{
	auto model_params = gl::model_loader_parameters{};
	model_params.file_format = ".tga";
	model_params.diffuse_texture_postfix = "_d";
	_textured = gl::model_from_file(model_path, model_params);

	_prog.from_file(shaded_shader_path);
	_cam = camera{radians(70.0f), aspect_ratio(), 0.01, 1000};
	_cam.look_at(vec3{1,0,0});
	_axis = gl::make_axis<mesh>();
	_light = gl::make_sphere<mesh>();
	_axis_prog.from_file(axis_shader_path);
	_light_prog.from_file(light_shader_path);
}

void scene_window::display()
{
	mat4 M = mat4{1};
	M = rotate(M, radians(-90.0f), vec3{1, 0, 0});
	mat4 world_to_camera = _cam.view();
	mat4 local_to_screen = _cam.view_projection() * M;
	mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera*M)};

	vec3 light_pos = vec3{5,5,-5};

	_prog.use();
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("world_to_camera", world_to_camera);
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_camera", normal_to_camera);
	_prog.uniform_variable("light.direction", normalize(light_pos));

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	_textured.render(_prog);

	// light
	mat4 M_light = translate(mat4{1}, light_pos);
	M_light = scale(M_light, vec3{0.1, 0.1, 0.1});
	_light_prog.use();
	_light_prog.uniform_variable("color", vec3{1,1,0});  // yellow
	_light_prog.uniform_variable("local_to_screen", _cam.view_projection() * M_light);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_light.render();

	// axis
	_axis_prog.use();
	_axis_prog.uniform_variable("local_to_screen", _cam.view_projection());

	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_axis.render();

	base::display();
}

void scene_window::input(float dt)
{
	_look.input(dt);
	_move.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
