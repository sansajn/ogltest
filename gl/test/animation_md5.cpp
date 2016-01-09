// md5 animacia
#include <string>
#include <vector>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/glut_window.hpp"
#include "gl/animation.hpp"
#include "gl/controllers.hpp"
#include "gl/program.hpp"

using std::vector;
using std::string;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::rotate;
using glm::inverseTranspose;
using ui::glut_pool_window;
using gl::animated_model;
using gl::free_camera;

string const mesh_path = "../assets/models/bob_lamp.md5mesh";
string const anim_path = "../assets/models/bob_lamp.md5anim";

// implementacia linear blend skinning-u
char const * skinning_shader_source = R"(
	const int MAX_JOINTS = 100;
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform vec3 color = vec3(0.5, 0.5, 0.5);
	uniform mat4 skeleton[MAX_JOINTS];  // kostra, ako zoznam transformacii

#ifdef _VERTEX_
	layout(location = 0) in vec3 position;  // bind pose positions
	layout(location = 2) in vec3 normal;
	layout(location = 4) in ivec4 joints;
	layout(location = 5) in vec4 weights;
	out vec3 n;

	void main()
	{
		mat4 T_skin =
			skeleton[joints.x] * weights.x +
			skeleton[joints.y] * weights.y +
			skeleton[joints.z] * weights.z +
			skeleton[joints.w] * weights.w;

		vec4 n_skin = T_skin * vec4(normal, 0);
		n = normal_to_world * n_skin.xyz;

		gl_Position = local_to_screen * T_skin * vec4(position, 1);
	}
#endif  // _VERTEX_

#ifdef _FRAGMENT_
	in vec3 n;
	out vec4 fcolor;
	vec3 light_direction = normalize(vec3(1,1,1));

	void main()
	{
		float light_intensity = clamp(dot(normalize(n), light_direction), 0.2, 1);
		fcolor = vec4(light_intensity * color, 1);
	}
#endif  // _FRAGMENT_
)";

class scene_window : public glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void update(float dt) override;
	void input(float dt) override;

private:
	animated_model _mdl;
	shader::program _prog;
	free_camera<scene_window> _view;
};

scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	auto model_params = gl::model_loader_parameters{};
	model_params.ignore_textures = true;
	_mdl = gl::animated_model_from_file(mesh_path, model_params);
	_mdl.append_animation(gl::skeletal_animation{anim_path});
	_mdl.animation_sequence(vector<unsigned>{0});

	_prog.from_memory(skinning_shader_source);
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
