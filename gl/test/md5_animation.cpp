// md5 animacia
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "window.hpp"
#include "animation.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "program.hpp"

using std::string;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::rotate;
using glm::inverseTranspose;
using ui::glut_pool_window;
using gl::animated_model;
using gl::camera;
using gl::free_look;
using gl::free_move;

string const mesh_path = "assets/models/bob_lamp.md5mesh";
string const anim_path = "assets/models/bob_lamp.md5anim";

string const skinning_shader_source = R"(
	// implementacia linear blend skinning-u
	const int MAX_JOINTS = 100;
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform vec3 color = vec3(0.5, 0.5, 0.5);
	uniform mat4 skeleton[MAX_JOINTS];  // kostra, ako zoznam transformacii

	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;  // bind pose positions
	layout(location = 1) in vec2 texcoord;
	layout(location = 2) in vec3 normal;
	// 3 for tangent
	layout(location = 4) in ivec4 joints;
	layout(location = 5) in vec4 weights;

	out VS_OUT {
		vec2 uv;
		vec3 normal;
	} vs_out;  // vertex shader outputs

	void main()
	{
		vs_out.uv = texcoord;

		mat4 T_skin =
			skeleton[joints.x] * weights.x +
			skeleton[joints.y] * weights.y +
			skeleton[joints.z] * weights.z +
			skeleton[joints.w] * weights.w;

		vec4 normal_ = T_skin * vec4(normal, 0);  // TODO: prejavy sa posunutie v nasobeni ?
		vs_out.normal = normal_to_world * normal_.xyz;

		gl_Position = local_to_screen * T_skin * vec4(position, 1);
	}
	#endif  // _VERTEX_
	#ifdef _FRAGMENT_
	in VS_OUT {
		vec2 uv;
		vec3 normal;
	} fs_in;  // fragment shader inputs

	out vec4 fcolor;

	vec3 light_direction = normalize(vec3(1,1,1));

	void main()
	{
		float light_intensity = clamp(dot(normalize(fs_in.normal), light_direction), 0.2, 1);  // simple lighting based on normals
		fcolor = vec4(light_intensity * color, 1);
	}
	#endif  // _FRAGMENT_
)";

class scene_window : public glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	~scene_window();
	void display() override;
	void update(float dt) override;
	void input(float dt) override;

private:
	animated_model _mdl;
	camera _cam;
	free_look<scene_window> _look;
	free_move<scene_window> _move;
	shader::program _prog;
	GLuint _vao;
};

scene_window::scene_window()
	: _look{_cam, *this}, _move{_cam, *this}
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	_mdl = gl::animated_model_from_file(mesh_path, anim_path);

	_cam = gl::camera{radians(70.0f), aspect_ratio(), 0.01, 1000};
	_cam.position = vec3{0,3,10};

	_prog.from_memory(skinning_shader_source);

	glClearColor(0, 0, 0, 1);
}

scene_window::~scene_window()
{
	glDeleteVertexArrays(1, &_vao);
}

void scene_window::display()
{
//	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	mat4 M = mat4{1};
	M = rotate(mat4{1}, radians(-90.0f), vec3{1,0,0});
	mat4 local_to_screen = _cam.view_projection() * M;
	mat3 normal_to_world = mat3{inverseTranspose(M)};

	_prog.use();
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_world", normal_to_world);
	_prog.uniform_variable("skeleton", _mdl.skeleton());
	_mdl.render();

	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);
	_mdl.update(dt);
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
