// animacia pomocou gles2 kompatibilneho shaderu \note zatial nefunguje
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/glut_window.hpp"
#include "gl/controllers.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/animation_gles2.hpp"

char const * skinned_shader_path = "skinned.glsl";
std::string model_path = "../../assets/models/bob_lamp";

using std::vector;
using std::string;
using glm::mat4;
using glm::mat3;
using glm::vec4;
using glm::vec3;
using glm::radians;
using glm::inverseTranspose;
using glm::normalize;
using glm::rotate;
using gles2::shader::program;
using gl::free_camera;
using gles2::animated_model;
using gles2::animated_model_from_file;
using gles2::skeletal_animation;
using gles2::model_loader_parameters;

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;
	scene_window();
	void display() override;
	void input(float dt) override;

private:
	program _skinned;
	free_camera<scene_window> _view;
	animated_model _mdl;
};

vector<vec4> create_matrix_palette(vector<mat4> const & skeleton);


scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01f, 1000.0f, *this}
{
	_skinned.from_file(skinned_shader_path);

	_view.get_camera().position = vec3{0,6,6};
	_view.get_camera().look_at(vec3{0,3.5,0});

	auto model_params = model_loader_parameters{};
	model_params.ignore_textures = true;
	_mdl = animated_model_from_file(model_path + ".md5mesh", model_params);
	_mdl.append_animation(skeletal_animation{model_path + ".md5anim"});
	_mdl.animation_sequence(vector<unsigned>{0});

	int position_a = _skinned.attribute_location("position");
	int texcoord_a = _skinned.attribute_location("texcoord");
	int normal_a = _skinned.attribute_location("normal");
//	int tangent_a = _prog.attribute_location("tangent");
	int joints_a = _skinned.attribute_location("indices");
	int weights_a = _skinned.attribute_location("weights");
	_mdl.attribute_location({position_a, texcoord_a, normal_a, joints_a, weights_a});
}

void scene_window::display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	gl::camera & cam = _view.get_camera();
	mat4 M = rotate(radians(-90.0f), vec3{1, 0, 0});
	mat4 world_to_screen = cam.world_to_screen();
	mat4 local_to_screen = world_to_screen * M;
	mat3 normal_to_camera = mat3{inverseTranspose(cam.world_to_camera() * M)};
	vec3 light_dir = normalize(vec3{1,2,3});

	_skinned.use();
	_skinned.uniform_variable("local_to_screen", local_to_screen);
	_skinned.uniform_variable("world_to_camera", cam.world_to_camera());
	_skinned.uniform_variable("normal_to_camera", normal_to_camera);
	_skinned.uniform_variable("light_direction", light_dir);

	vector<vec4> palette = create_matrix_palette(_mdl.skeleton());  // toto sa meni v case
	if (palette.size() > 32*3)
	{
		std::cerr << "matrix-palette is too big " << palette.size() << " vs 96" << std::endl;
		palette.resize(32*3);  // shader spadne
	}

	_skinned.uniform_variable("matrix_palette", palette);

	_mdl.render(_skinned);

	base::display();
}

void scene_window::input(float dt)
{
	_view.input(dt);
	base::input(dt);
}

vector<vec4> create_matrix_palette(vector<mat4> const & skeleton)
{
	vector<vec4> result;
	result.reserve(skeleton.size() * 3);
	for (mat4 const & M : skeleton)
	{
		result.emplace_back(M[0].x, M[1].x, M[2].x, M[3].x);
		result.emplace_back(M[0].y, M[1].y, M[2].y, M[3].y);
		result.emplace_back(M[0].z, M[1].z, M[2].z, M[3].z);
	}
	return result;
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
