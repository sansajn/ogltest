// md5 animacia
#include <string>
#include <vector>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/glfw3_window.hpp"
#include "gl/free_camera.hpp"
#include "gl/colors.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/animation_gles2.hpp"

using std::vector;
using std::string;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::rotate;
using glm::inverseTranspose;
using gl::free_camera;
using gles2::animated_model;
using gles2::shader::program;
using gles2::model_loader_parameters;
using gles2::animated_model_from_file;
using gles2::skeletal_animation;

string const mesh_path = "../../assets/models/bob_lamp/bob_lamp.md5mesh";
string const anim_path = "../../assets/models/bob_lamp/bob_lamp.md5anim";

// implementacia linear blend skinning-u
char const * skinning_shader_source = R"(
#ifdef _VERTEX_
	const int MAX_JOINTS = 100;
	
	attribute vec3 position;  // bind pose positions
	attribute vec3 normal;
	attribute vec4 joints;  // skeleton indices
	attribute vec4 weights;
	
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform mat4 skeleton[MAX_JOINTS];  // kostra, ako zoznam transformacii
	
	varying vec3 n;

	void main()
	{
		mat4 T_skin =
			skeleton[int(joints.x)] * weights.x +
			skeleton[int(joints.y)] * weights.y +
			skeleton[int(joints.z)] * weights.z +
			skeleton[int(joints.w)] * weights.w;

		vec4 n_skin = T_skin * vec4(normal, 0);
		n = normal_to_world * n_skin.xyz;

		gl_Position = local_to_screen * T_skin * vec4(position, 1);
	}
#endif  // _VERTEX_

#ifdef _FRAGMENT_
	precision mediump float;
	uniform vec3 color;
	varying vec3 n;
	vec3 light_direction = normalize(vec3(1,2,3));

	void main()
	{
		float light = clamp(dot(normalize(n), light_direction), 0.2, 1.0);
		gl_FragColor = vec4(light * color, 1);
	}
#endif  // _FRAGMENT_
)";


class scene_window : public ui::glfw_window
{
public:
	using base = ui::glfw_window;

	scene_window();
	void display() override;
	void update(float dt) override;
	void input(float dt) override;

private:
	animated_model _mdl;
	program _prog;
	free_camera<scene_window> _view;
};

scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_prog.from_memory(skinning_shader_source);

	auto model_params = model_loader_parameters{};
	model_params.ignore_textures = true;
	_mdl = animated_model_from_file(mesh_path, model_params);
	_mdl.append_animation(skeletal_animation{anim_path});
	_mdl.animation_sequence(vector<unsigned>{0});

	int position_a = _prog.attribute_location("position");
	int texcoord_a = _prog.attribute_location("texcoord");
	int normal_a = _prog.attribute_location("normal");
//	int tangent_a = _prog.attribute_location("tangent");
	int joints_a = _prog.attribute_location("joints");
	int weights_a = _prog.attribute_location("weights");
	_mdl.attribute_location({position_a, texcoord_a, normal_a, joints_a, weights_a});

	_view.get_camera().position = vec3{0,3,10};

	glClearColor(0, 0, 0, 1);
}

void scene_window::display()
{
//	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	mat4 M = mat4{1};
	M = rotate(mat4{1}, radians(-90.0f), vec3{1,0,0});
	mat4 local_to_screen = _view.get_camera().view_projection() * M;
	mat3 normal_to_world = mat3{inverseTranspose(M)};

	_prog.use();
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_world", normal_to_world);
	_prog.uniform_variable("skeleton", _mdl.skeleton());
	_prog.uniform_variable("color", rgb::olive);
	_mdl.render(_prog);

	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);
	_mdl.update(dt);
}

void scene_window::input(float dt)
{
	_view.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
